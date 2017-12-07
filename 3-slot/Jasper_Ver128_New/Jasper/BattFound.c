//-----------------------------------------------------------------------------
// FILENAME:  battfound.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares functions to validate battery EEPROM/Gas Gauge data, battery authentication and EEPROM updates
//				
//
// %IF Zebra_Internal
//
// NOTES:    
//
// AUTHOR:   		Wasath Mudalige
// CREATION DATE: 	03/25/2016
// DERIVED FROM: 	New File
//
// EDIT HISTORY:
//
//
//
// %End 
//-----------------------------------------------------------------------------

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "a2d.h"
#include "led.h"
#include "uart.h"
#include "debug.h"
#include "macros.h"
#include "eeprom.h"
#include "timers.h"
#include "BattAuth.h"
#include "version.h"
#include "thermdefs.h"
#include "i2c_batt.h"
#include "battchrg.h"
#include "battcomm.h"
#include "sysparam.h"
#include "batt_defs.h"
#include "battfound.h"
#include "jasper_ports.h"
#include "PP_BattDefs.h"

uint8_t zebra_seed[] = {0x1C, 0x04, 0x04, 0x00};
uint8_t CommBuf[MAX_AUTH_CMD_SIZE];	// This is a general purpose buffer
//-----------------------------------------------------------------------------
//
// Prototype:	CheckBattFound
//
// Description:	Routine to check the battery found
//
// Parameters:	None
//
// Returns:		Battery found error
// 
// Notes: 		This routine shall set BATT_PRESET_FLAG upon successful detection
//-----------------------------------------------------------------------------
uint8_t CheckBattFound(void)
{
	A2D_P *pa2d;
	pa2d = (A2D_P*)&A2D + PORTR_OUT;
	
	// NOTE: Bellow I2C states are NOT in order. Please refer BATT_I2C_STATES enum in "BattComm.h"
	switch (pBatt->I2CState)
	{
		// ---------------------------- Toaster Firmware update checking starts here ------------------------------
		case READ_ID_FROM_FUB:
			CommBuf[0] = ID_BLOCK_ADDR_LO;					// High capacity EEPROM requires 2 byte to select the read address (0x3FF0)
			AccessBattChip(FUB_SLV_ADDR, ID_BLOCK_ADDR_HI, FUB_BLOCK_SIZE, 1, CommBuf);	// Read the very last block of high capacity EEPROM 
			break;
			
		case VERIFY_DEV_ID_FW_VER:
		{
			uint8_t curr_slot = PORTR_OUT;
			uint8_t dev_id = CommBuf[OFFSET_DEV_ID];
			uint8_t major_ver = CommBuf[OFFSET_VER_MAJOR];
			uint8_t minor_ver = CommBuf[OFFSET_VER_MINOR];

			if (SLOT0 == curr_slot)					// If FUB is inserted into slot0
			{
				if (JASPER_3SLOT_ID == dev_id)		// If Toaster Type matches?
				{// Yes
					if ((major_ver != VERSION_MAJOR) || (minor_ver != VERSION_MINOR)) 	// If new firmware version is different from current firmware version?
					{
						SYS.ModeReg = ENTER_BL_bm;					// Enter into boot loader
						SYS.SkipSleep = TRUE;
						break;
					}
					else
						LED.Pat.Slot[SLOT0].Grn = SLOW_BLINK_PAT;	// Firmware update successful, notify the user with Green slow blink pattern	
				}
				else
					LED.Pat.Slot[SLOT0].Red = SLOW_BLINK_PAT;		// This firmware is not for Jasper 3-slot. Notify the user with Red slow blink pattern
			}
			else
			{ 
				if (SLOT2 == curr_slot)					// If FUB is inserted into slot2
				{
					major_ver = VERSION_MAJOR;			// Start showing toaster FW version
					minor_ver = VERSION_MINOR;
				}
				else
				{										// If FUB is inserted into slot1
					minor_ver = 0;						// Just do Amber LED blinking
					major_ver = 9;
					SYS.FUBMemAddr = 0;					// Clear FUB memory address
				}
					
				major_ver = major_ver % 10;				// Get the LSD of the major version
				if (major_ver)							// If major version is non zero
				{// Yes
					pBatt->FWMajorVer = major_ver;		// Store the FW versions in unused variables in BATT structure
					pBatt->FWMinorVer = minor_ver;
					pBatt->PWMSettleCnt = START_SHOW_FW_VER;	// Time to start LED blinking
					pBatt->WaitCnt = 0;					// Clear wait count
				}
			}
			pBatt->BattType = BATT_TYPE_FUB;		// Set battery type as FUB
			pBatt->I2CState = SIMPLY_WAIT;			// Set next i2c state
			break;
		}
		
		case WRITE_FUB_PAGE:
			pEEP->HealthThreshold ^= 1;			// Just to Toggle Red and Green LEDs
			SetChrgLEDPat(CHARGING_COMPLETE);		// Start LEDs
			AccessBattChip(FUB_SLV_ADDR, SYS.FUBDataBuf[0], 0, (FUB_PAGE_SIZE+1), &SYS.FUBDataBuf[1]);	// Write encrypted data in FUB
			break;
		
		case WAIT_FUB_PAGE_WRITE:
			Wait_mSecs(CNT_8MS);					// Wait 8ms to complete FUB page write (must wait at least 5ms)
			break;
		
		case DO_NEXT_FUB_PAGE_WRITE:
			SYS.FUBMemAddr += FUB_PAGE_SIZE;		// Move to next page address
			if (!SYS.FUBMemAddr)
			{
				memcpy(SYS.FUBDataBuf, FRAME.Data, (FUB_PAGE_SIZE+2));	// Get a copy of FUB data
				pBatt->I2CState = WRITE_FUB_PAGE;
				SYS.SkipSleep = TRUE;
				break;
			}
			if (MAX_FUB_MEM_ADDR == SYS.FUBMemAddr)	// If all the pages are written
			{
				SYS.FUBMemAddr = 0;					// Reset the memory address
				pEEP->HealthThreshold = 0;			// Slow Solid Green LEDs
				SetChrgLEDPat(CHARGING_COMPLETE);	// Start LEDs
			}
			pBatt->I2CState = SIMPLY_WAIT;			// Good to write next FUB page
			SendRS232(ACK);							// Send the acknowledgment
			break;
		
		// --------------------------------------------------------------------------------------------------------
		
		// -------------Dead battery detection/ charging starts here ----------------------------------------------
		case CHECK_DEAD_BATT_CHRG:
			if (pa2d->BattVSense < BAT_LOW_LEVEL)		// If battery voltage is too low to continue
			{
				SetBatteryPresetFlag(BATT_TYPE_DEAD);		// Start dead battery charging
				break;
			}
			pBatt->I2CState = CHECK_DEAD_BATT_CHRG2;
			SetChrgLEDPat(CHARGING_ON_PROGRESS);			// Set charging on progress LED pattern	
			// No "break" here
		
		case CHECK_DEAD_BATT_CHRG2:
			pBatt->Volt_PWM_Off = pa2d->BattVSense;			// Remember the initial voltage when FETs are on
			pBatt->WaitCnt = 30;
			pBatt->I2CState = CHECK_DEAD_BATT_CHRG3;
			ShutdownCharger();								// Turn off FETs
			pBatt->ChrgState = CHRG_PAUSE;
			break;
			
		case CHECK_DEAD_BATT_CHRG3:
			if (!pBatt->WaitCnt)							// If three seconds elapsed?
			{
				int16_t diff;
				if (pBatt->Volt_PWM_Off >= pa2d->BattVSense)	// Calculate the voltage difference between FETs are on/off
					diff = pBatt->Volt_PWM_Off - pa2d->BattVSense;
				else
					diff = pa2d->BattVSense - pBatt->Volt_PWM_Off;
				
				PORTA_OUTCLR = (CHG_DIS0_bm << PORTR_OUT);		// Turn on fuse FETs
				pBatt->ChrgState = REQUEST_TO_CHRG;
					
				if (diff > 250)									// If the difference is beyond 250mV
					{
						//SendRS232('X');
						SetBatteryPresetFlag(BATT_TYPE_DEAD);}		// Start dead battery charging
				else
					{
					//SendRS232('Y');
					pBatt->I2CState = READ_DEV_NAME_QLN;}		// Battery voltage is stable, good to carry on with gas gauge reading
			}
			else
				{
				//SendRS232('Z');
				Wait_mSecs(CNT_10MS);}
			break;
			
		case DONE_DEAD_BATT_CHRG:
			if (ACA_STATUS & AC_AC0STATE_bm)		// If the battery thermistor was disconnected
				pBatt->I2CState = BATT_REMOVED;		// Declare it as battery has been removed
			else
			{
				Gauge.Volt = pa2d->BattVSense;
				Gauge.AvgCurr = 0;
				Gauge.GGInstCurr = 0;
				Gauge.InstCurr = 0;
				Gauge.Status[0] = 0;
				Gauge.Status[1] = 0;
				pEEP->ColdOff = 0;
				pEEP->HotOff = DEAD_HOT_OFF_TMP;
				pBatt->Tmp = DEF_ROOM_TMP;
				pBatt->SOC = 0;
				pBatt->SOH = 100;
				DoBattChrg(CHECK_DEAD_BATT_CHRG3);	// Call battery charger and set next i2c state
			}
			break;
		// --------------------------------------------------------------------------------------------------------
		
		// ------------------------------ PP battery found starts here --------------------------------------------	
		case READ_DEV_NAME_QLN:						// Read device name
			memset(CommBuf, 0, PN_SIZE);
			AccessBattChip(GG_SLV_ADDR_QLN, DEVICE_NAME, BLOCK_READ, 0, CommBuf);
			break;
			
		case VERIFY_DEV_NAME_QLN:					// Verify device name
		{
			DEBUG_DATA *pDD;
			pDD = DD + PORTR_OUT;
			memcpy(pDD->PartNum, CommBuf, PN_SIZE);
			
			// Check for Rolex battery
			if (!memcmp(CommBuf, ROLEX_DEV_NAME, strlen(ROLEX_DEV_NAME)))
			{
				pBatt->BattType = BATT_TYPE_RLX1;	
			}
			else
			{
				// Check for QLN common device name
				if (memcmp(CommBuf, QLN_DEV_NAME, strlen(QLN_DEV_NAME)))
					return BATT_UNSUPPORTED;

				pBatt->BattType = BATT_TYPE_QLN;
			}
			pEEP->CCThreshold = DEF_CC_THRESHOLD;
			pBatt->I2CState = READ_CHRG_CURR_QLN;
			// No "break" here
		}
		
		case READ_CHRG_CURR_QLN:
			AccessBattChip(GG_SLV_ADDR_QLN, CHARGING_CURRENTS, 2, 0, (uint8_t*)&pEEP->FastChrgCurr);
			break;
		
		case READ_DESIGN_CAP_QLN:
			AccessBattChip(GG_SLV_ADDR_QLN, DESIGN_CAP_QLN, 2, 0, (uint8_t*)&pBatt->DesignCap);
			break;
		
		case READ_DATE_MFG_QLN:
			AccessBattChip(GG_SLV_ADDR_QLN, MFG_DATE, 2, 0, &CommBuf[0]);
			break;
			
		case READ_BATT_SN_QLN:
			AccessBattChip(GG_SLV_ADDR_QLN, SERIAL_NUM, 2, 0, &CommBuf[2]);
			break;
			
		case READ_CC_THRESHOLD_QLN:
			AccessBattChip(GG_SLV_ADDR_QLN, CC_THRESHOLD, 2, 0, (uint8_t*)&pEEP->CCThreshold);
			break;
				
		case SET_BATT_PRESENT_FLAG:
		{
			DEBUG_DATA *pDD;
			uint16_t year = (CommBuf[1]/2) + 1980;
			uint8_t month = ((CommBuf[1] & 0x01)<<3) | ((CommBuf[0]&0xE0) >> 5);
			pDD = DD + PORTR_OUT;
			pDD->DateMfg[0] =  CommBuf[0]%32;
			pDD->DateMfg[1] = (year&0xFF);
			pDD->DateMfg[2] = ((month << 4) | (year >> 8));
			
			pDD->SerialNum[0] = HexToAscii(CommBuf[2] & 0xF);
			pDD->SerialNum[1] = HexToAscii((CommBuf[2]>>4) & 0xF);
			pDD->SerialNum[2] = HexToAscii(CommBuf[3] & 0xF);
			pDD->SerialNum[3] = HexToAscii((CommBuf[3]>>4) & 0xF);
			pDD->SerialNum[4] = 0;
			pDD->SerialNum[5] = 0;
			
			if (BATT_TYPE_QLN == pBatt->BattType)
			{
				SetBatteryPresetFlag(BATT_TYPE_QLN);
				break;
			}
			pBatt->I2CState = AUTH_INIT;
		}
		// ------------- PP+ battery authentication starts here (ATECC508A) -------------------------
		// IMPORTANT NOTE: We have no idea whether the watchdog timer is about the expire in battery auth chip
		// Therefore put the battery auth chip into sleep mode first so that it ensures watchdog timer gets reset
		// This I2C call could be NACKed depending on previous usage of the battery
		case AUTH_INIT:
			memset(&AUTH, 0, sizeof(AUTH_DATA));
			AUTH.ExecTime = AUTH_SLEEP_DELAY;
			AUTH.I2CSlvAddr = AUTH_SLV_ADDR_MLB;
			AccessBattChip(AUTH_SLV_ADDR_MLB, AUTH_SLEEP_REG, 0, 0, 0);	// Put the MLB auth chip into sleep mode
			break;
			
		case AUTH_PUT_SLEEP_ZQ3:
			AUTH.I2CSlvAddr = AUTH_SLV_ADDR_ZQ3;
			AccessBattChip(AUTH_SLV_ADDR_ZQ3, AUTH_SLEEP_REG, 0, 0, 0);// Put the ZQ3 auth chip into sleep mode
			break;
			
		case AUTH_PUT_SLEEP_RLX:
			if (!AUTH.BattSlvAddr)	// If the ZQ3 battery didn't response to previous sleep call
			{// Try Rolex battery
				AUTH.I2CSlvAddr = AUTH_SLV_ADDR_RLX;
				AccessBattChip(AUTH_SLV_ADDR_RLX, AUTH_SLEEP_REG, 0, 0, 0);// Put the RLX auth chip into sleep mode
				break;
			}
			else
				pBatt->I2CState = AUTH_WAKE;
			// No "break" here
			
		case AUTH_WAKE:
			AccessBattChip(0x00, 0x00, 0, 0, 0);		// Wakeup the auth chips. This will be NACKed!
			break;
		
		case AUTH_READ_ENCRYPTED:
			if (!((1 == AUTH.RespLen) && (CMD_STATUS_SUCCESS == AUTH.RespBuf[0])))
				return AUTH_ERR_GEN_DIGEST;
			// No "break" here
			
		case AUTH_READ_CHRG_DATA:
		case AUTH_READ_SLOT_DATA:
			AuthReadData();								// Read 32 bytes from selected slot/block
			break;
		
		case AUTH_COPY_READ_ENCRYPTED:
		case AUTH_COPY_READ_DATA:
		{
			uint8_t offset = 0;
			uint8_t a=0;
			uint8_t b=0;
			uint8_t c=0;
			uint8_t d=0;
			uint8_t sig = FALSE;

			if (AUTH.RespLen != ATCA_BLOCK_SIZE)		// If the response size is not correct
				return AUTH_ERR_READ_SLOT_DATA;			// Error reading device public key (part1)
			
			pBatt->I2CState = AUTH_READ_SLOT_DATA;
				
			switch (AUTH.SlotAddrLo)
			{
				case ADDR_SLOT0:  
					// Verify serial number
					if ((AUTH.RespBuf[0] != AUTH_SN0) || (AUTH.RespBuf[1] != AUTH_SN1) || (AUTH.RespBuf[12] != AUTH_SN8))
						return AUTH_ERR_VERIFY_SN;				// Device auth chip serial number verification failed
					AUTH.SlotAddrLo = ADDR_SLOT9;
					SYS.SkipSleep = TRUE;
					return NO_ERROR;
					
				case ADDR_SLOT9:  offset = 0; break;
				case ADDR_SLOT10: offset = 1; sig = TRUE; break;
				case ADDR_SLOT11: offset = 2; break;
				case ADDR_SLOT12: offset = 3; sig = TRUE; break;
				case ADDR_SLOT13: offset = 4; sig = TRUE; break;
				case ADDR_SLOT14: offset = 5; sig = TRUE; break;
			}

			switch (AUTH.SlotAddrHi)
			{
				case ADDR_BLK0:
					if (sig)
					{
						a = 0;
						b = 32;
					}
					else
					{
						c = 4;
						d = 28;
					}
					break;
					
				case ADDR_BLK1:
					if (sig)
					{
						a = 32;
						b = 32;
						if (offset == 3)
							pBatt->I2CState = AUTH_GEN_RANDON_NUMBER;
					}
					else
					{
						a = 28;
						b = 4;
						c = 8;
						d = 24;
					}
					break;
					
				case ADDR_BLK2:
					a = 56;
					b = 8;
					break;
			}
				
			uint8_t *buf_ptr = AUTH.ClientPubKey + (uint16_t)offset*KEY_LEN + a;
			if (AUTH.I2CSlvAddr == AUTH_SLV_ADDR_MLB)
			{
				for (uint8_t n=0; n<ATCA_BLOCK_SIZE; n++)
				{
					*buf_ptr ^= AUTH.RespBuf[n];
					buf_ptr++;
				}
				
				if ((a == 32) && (offset == 5))
				{
					AUTH.SlotAddrLo = ADDR_SLOT10;
					AUTH.SlotAddrHi = ADDR_BLK0;
					pBatt->I2CState = AUTH_SETUP_SLOT_DATA;
					break;
				}
				
				AUTH.I2CSlvAddr = AUTH.BattSlvAddr;
				pBatt->I2CState = AUTH_GEN_RANDON_NUMBER;
			}
			else
			{
				CopyResp(buf_ptr, 0, b);
				CopyResp((buf_ptr+b), c, d);
				
				if (offset >= 4)
				{
					pBatt->I2CState = AUTH_NONCE_MLB;
					break;
				}
			}
			
			if ((a == 56) || (a == 32))
			{
				AUTH.SlotAddrHi = ADDR_BLK0;
				AUTH.SlotAddrLo += 0x08;
			}
			else
				AUTH.SlotAddrHi++;	
		}
		break;
		
		case AUTH_GEN_RANDON_NUMBER:
			memset(AUTH.RandomNum, 0, RANDOM_NUM_SIZE);
			AuthNouce(/*pBatt->I2CSlvAddr*/AUTH.BattSlvAddr, NONCE_MODE_SEED_UPDATE);
			break;
			
		case AUTH_COPY_RND_NUM:
			if (AUTH.RespLen != RANDOM_NUM_SIZE)
				return AUTH_ERR_GEN_RND_NUM;
			CopyResp(AUTH.RandomNum, 0, RANDOM_NUM_SIZE);
			pBatt->I2CState = AUTH_GEN_DIGEST;
			// No "break" here
			
		case AUTH_GEN_DIGEST:
			AuthGenDig();
			break;
			
		case AUTH_NONCE_MLB:
			AuthNouce(AUTH_SLV_ADDR_MLB, NONCE_MODE_PASSTHROUGH);
			break;
			
		case AUTH_DIGEST_RND_NUM:
			if (!((1 == AUTH.RespLen) && (CMD_STATUS_SUCCESS == AUTH.RespBuf[0])))
				return AUTH_ERR_NONCE_MLB;
			AuthDigestRndNum();
			break;
			
		case AUTH_DERIVE_KEY:
			if (AUTH.RespLen != ATCA_BLOCK_SIZE)
				return AUTH_ERR_DIGEST_RAD_NUM;
			AuthDeriveKey();
			break;
			
		case AUTH_READ_SLOT4_MLB:
			if (!((1 == AUTH.RespLen) && (CMD_STATUS_SUCCESS == AUTH.RespBuf[0])))
				return AUTH_ERR_DRIVE_KEY;
			AUTH.I2CSlvAddr = AUTH_SLV_ADDR_MLB;
			pBatt->I2CState = AUTH_READ_SLOT_DATA;
			break;
			
		case AUTH_SETUP_SLOT_DATA:
		{
			uint8_t offset = 0;
			uint8_t a=0;
			uint8_t b=0;
			uint8_t c=0;
			uint8_t d=0;
			
			switch (AUTH.SlotAddrHi)
			{
				case ADDR_BLK0:
					c = 4;
					d = 28;
					break;
				
				case ADDR_BLK1:
					a = 28;
					b = 4;
					c = 8;
					d = 24;
					break;
				
				case ADDR_BLK2:
					a = 56;
					b = 8;
					break;
			}
			
			if (AUTH.SlotAddrLo == ADDR_SLOT12)
				offset = 2*KEY_LEN;

			uint8_t *buf_ptr = AUTH.ClientPubKey + offset + a;
			memset(AUTH.RandomNum, 0, RANDOM_NUM_SIZE);
			memcpy(&AUTH.RandomNum[0], buf_ptr, b);
			memcpy(&AUTH.RandomNum[c], (buf_ptr+b), d);
			pBatt->I2CState = AUTH_WRITE_SLOT_DATA;
		}
		// No "break" here
		
		case AUTH_WRITE_SLOT_DATA:
			AuthWriteData();
			break;
			
		case AUTH_CHECK_WRITE_STATUS:
			if (!((1 == AUTH.RespLen) && (CMD_STATUS_SUCCESS == AUTH.RespBuf[0])))
				return AUTH_ERR_WRITE_SLOT;
			
			if (++AUTH.SlotAddrHi > ADDR_BLK2)
			{
				if (AUTH.SlotAddrLo == ADDR_SLOT12)
				{
					AUTH.SlotAddrHi = KEY_ID12;
					AUTH.SlotAddrLo = KEY_LEN;
					pBatt->I2CState = AUTH_SHA256_START;
					break;
				}
				AUTH.SlotAddrHi = ADDR_BLK0;
				AUTH.SlotAddrLo += 0x10;
			}
			pBatt->I2CState = AUTH_SETUP_SLOT_DATA;
			break;
		
		case AUTH_SHA256_START:
			AuthSHA256(SHA_MODE_SHA256_START, 0, 0);
			break;
			
		case AUTH_SHA256_PUBLIC:
			if (!((1 == AUTH.RespLen) && (CMD_STATUS_SUCCESS == AUTH.RespBuf[0])))
				return AUTH_ERR_CMD_SHA256;
			AuthSHA256(SHA_SHA256_PUBLIC_MASK, AUTH.SlotAddrHi, 0);
			break;
			
		case AUTH_SHA256_UPDATE:
			if (!((1 == AUTH.RespLen) && (CMD_STATUS_SUCCESS == AUTH.RespBuf[0])))
				return AUTH_ERR_CMD_SHA256;
			AuthSHA256(SHA_MODE_SHA256_UPDATE, EXTRA_LEN, (AUTH.ClientExtra + AUTH.SlotAddrLo));
			break;
			
		case AUTH_SHA256_END:
			if (!((1 == AUTH.RespLen) && (CMD_STATUS_SUCCESS == AUTH.RespBuf[0])))
				return AUTH_ERR_CMD_SHA256;
			AuthSHA256(SHA_MODE_SHA256_END, 0, 0);
			break;
			
		case AUTH_VERIFY_EXTERN:
			AuthVerifyExtern(AUTH.SlotAddrHi, (AUTH.ClientSignat + 2*AUTH.SlotAddrLo));
			break;
			
		case AUTH_CHECK_STATUS:
			if (!((1 == AUTH.RespLen) && (CMD_STATUS_SUCCESS == AUTH.RespBuf[0])))
				return AUTH_ERR_VERIFY_SIG_CERT;			// Failed to verify signer certificate
			if (AUTH.SlotAddrHi == KEY_ID12)
			{
				AUTH.SlotAddrHi = KEY_ID10;
				AUTH.SlotAddrLo = 0;
				pBatt->I2CState = AUTH_SHA256_START;
			}
			else
			{
				AUTH.I2CSlvAddr = AUTH.BattSlvAddr;
				AUTH.SlotAddrLo = ADDR_SLOT7;
				AUTH.SlotAddrHi = ADDR_BLK0;
				pBatt->I2CState = AUTH_READ_CHRG_DATA;
			}
			break;
		
		case AUTH_VERIFY_CHRG_DATA:
			if (AUTH.RespLen != ATCA_BLOCK_SIZE)		// If the response size is not correct
				return AUTH_ERR_READ_SLOT8;				// Return with error reading slot8 data
			DEBUG_DATA *pDD;
			pDD = DD + PORTR_OUT;	
			if (ADDR_BLK0 == AUTH.SlotAddrHi)
			{
				if (ADDR_SLOT7 == AUTH.SlotAddrLo)		// If reading health threshold
				{
					// If reading SLOT7/ BLOCK0 (Battery P/N)
					if (AUTH.RespBuf[0] == 0x00)						// If record type is zero (which is supported currently)
						pEEP->HealthThreshold = AUTH.RespBuf[1];		// Read the value
					else
						pEEP->HealthThreshold = DEF_HEALTH_THRESHOLD;	// Otherwise use the default value	
						
					if ((!pEEP->HealthThreshold) || (pEEP->HealthThreshold > 100))	// If health threshold is not defined
						pEEP->HealthThreshold = DEF_HEALTH_THRESHOLD;				// Use the default value
						
					AUTH.SlotAddrLo = ADDR_SLOT8;						// Next read slot8/block0 data
					pBatt->I2CState = AUTH_READ_CHRG_DATA;
				}
				else
				{	// If reading SLOT8/ BLOCK0 (Battery P/N)
					memcpy(pDD->PartNum, &AUTH.RespBuf[12], PN_SIZE);	// Copy battery P/N
					AUTH.SlotAddrHi = ADDR_BLK1;						// Next read slot8/block1 data
					pBatt->I2CState = AUTH_READ_CHRG_DATA;
				}
				break;
			}
			
			if (ADDR_BLK1 == AUTH.SlotAddrHi)
			{
				memcpy(pDD->SerialNum, &AUTH.RespBuf[2], SN_SIZE);		// Copy battery S/N
				AUTH.SlotAddrHi = ADDR_BLK4;							// Next read slot8/block4 data
				pBatt->I2CState = AUTH_READ_CHRG_DATA;
				break;
			}
				
			if (ADDR_BLK4 == AUTH.SlotAddrHi)
			{
				if (AUTH_SLV_ADDR_RLX == AUTH.BattSlvAddr)
				{
					uint8_t slot8_blank = TRUE;
					for (uint8_t n=0; n<AUTH.RespLen; n++)
					{
						if (AUTH.RespBuf[n] != 0xFF)
						{
							slot8_blank = FALSE;
							break;
						}
					}
					if (slot8_blank)
					{
						SetBatteryPresetFlag(BATT_TYPE_RLX0);
						break;
					}
				}
				
				// Slot 8 has data. Now verify them
				if (AUTH.RespBuf[DATA_REV] != SUPPORTED_DATA_REV)
					return INVALID_BATT_DATA_REV;
				
				if (AUTH.RespBuf[PP_VERSION] != SUPPORTED_PP_VER)
					return INVALID_PP_VERSION;
				
				pBatt->DesignCap = LITTLE_ENDIAN_INT(DESIGN_CAP);
				pEEP->SlowChrgCurr = _200mA;
				pEEP->FastChrgCurr = LITTLE_ENDIAN_INT(MAX_CHRG_RATE);
				pEEP->AbnormalChrgCurr = pEEP->FastChrgCurr + 500;
				pEEP->RechrgThreshold = AUTH.RespBuf[RECHRG_THRESHOLD];
				pEEP->ColdOn = LITTLE_ENDIAN_INT(CHRG_TMP_LOW)/10 - KELVIN_CONST;
				pEEP->ColdOff = pEEP->ColdOn - 2;
				pEEP->HotOn = LITTLE_ENDIAN_INT(CHRG_TMP_HIGH)/10 - KELVIN_CONST;
				if (pEEP->HotOn > 45)
					pEEP->HotOn = 45;
				pEEP->HotOff = pEEP->HotOn + 2;
				pEEP->SlowChrgMins = AUTH.RespBuf[SLOW_TIMEOUT] * 60;
				pEEP->FastChrgMins = ((uint16_t)AUTH.RespBuf[FAST_TIMEOUT] * 60);
				pEEP->ChrgupVolt = LITTLE_ENDIAN_INT(SCL_VOLT1)* 10;
				pEEP->SlowFastVolt = LITTLE_ENDIAN_INT(PRE_CHRG_VOLT);
				
				pEEP->MaxTemp1 = LITTLE_ENDIAN_INT(SCL_TEMP1)/10 - KELVIN_CONST;
				pEEP->FastChrgCurr1 = LITTLE_ENDIAN_INT(SCL_RATE1);
				pEEP->ChrgupVolt1 = LITTLE_ENDIAN_INT(SCL_VOLT1)*10;
				
				pEEP->MaxTemp2 =  LITTLE_ENDIAN_INT(SCL_TEMP2)/10 - KELVIN_CONST;
				pEEP->FastChrgCurr2 = LITTLE_ENDIAN_INT(SCL_RATE2);
								
				AUTH.SlotAddrHi = ADDR_BLK5;
				pBatt->I2CState = AUTH_READ_CHRG_DATA;
				break;
			}
			
			pEEP->ChrgupVolt2 = LITTLE_ENDIAN_INT(SCL_VOLT2)*10;
			
			pEEP->MaxTemp3 =  LITTLE_ENDIAN_INT(SCL_TEMP3)/10 - KELVIN_CONST;
			pEEP->ChrgupVolt3 = LITTLE_ENDIAN_INT(SCL_VOLT3)*10;
			pEEP->FastChrgCurr3 = LITTLE_ENDIAN_INT(SCL_RATE3);
			
			pEEP->ChrgTermCurr = AUTH.RespBuf[CHRG_TERM_CURR];
			pEEP->NearlyDoneCurr = pEEP->ChrgTermCurr + 100;
			if (BATT_TYPE_RLX1 == pBatt->BattType)
				SetBatteryPresetFlag(BATT_TYPE_RLX1);
			else		
				SetBatteryPresetFlag(BATT_TYPE_ZQ3);
			break;
		//----------------------------------------------------------------------------------------------------------
			
		//---------------------------- Dead battery charging starts here -------------------------------------------
		case SET_A2D_MUXES_DEAD:
			if (SetA2DMuxForCurrTmp())
				Wait_mSecs(A2D_READ_DELAY);				// Wait 15mS before taking A2D readings	
			break;
			
		case READ_A2D_CURR_TMP_DEAD:
			if (ACA_STATUS & AC_AC0STATE_bm)		// If the battery thermistor is disconnected
			{
				pBatt->I2CState = COMM_FAULT;		// Declare it as battery has been removed
				break;
			}
			if (ReadA2DCurrTmp(JUST_SLOW_CHRG_DEAD))	// Take A2D readings
				break;
		
		case JUST_SLOW_CHRG_DEAD:
			Gauge.InstCurr = (pa2d->ISenseConst * (int32_t)A2D.BattISense)/((uint16_t)16*A2D_RES);
			Gauge.AvgCurr = Gauge.InstCurr;
			Gauge.GGInstCurr = Gauge.InstCurr;
			Gauge.Volt = pa2d->BattVSense - (Gauge.InstCurr/100);;				// Set pack voltage
			Gauge.Status[0] = 0;
			Gauge.Status[1] = 0;
			pBatt->SOC = 0;								// Clear battery SOC
			pBatt->SOH = 100;
			pBatt->Tmp = DEF_ROOM_TMP;					// Just assume temperature is 25'C for the moment
			
			uint8_t next_i2c_state = SET_A2D_MUXES_DEAD;// Set next i2c state
			
			if (DO_SLOW_CHRG == pBatt->ChrgState) 		// If battery is slow charging
			{
				if (!pBatt->DeadBattChrgCnt)			// If dead battery has been charged at least one minute
				{									
					if (Gauge.Volt > BAT_LOW_LEVEL) 		// If the battery voltage is good enough to talk to the gas gauge
					{
						pEEP->SlowChrgMins = (pBatt->ChrgTimeSecs/60);
						next_i2c_state = CHECK_DEAD_BATT_CHRG2;	// Good to continue with battery verification	
						pBatt->MiscFlags &= ~FIXED_PWMI_bm;		// Clear pwmi fixed flag in case it was set during dead battery charging					
					}
				}
			}

			DoBattChrg(next_i2c_state);					// Call battery charger and set next i2c state
			SYS.SkipSleep = TRUE;
			return (pBatt->Fault);
		// ---------------------------------------------------------------------------------------------------------
			

		
	}
	SYS.SkipSleep = TRUE;
	return NO_ERROR;
}

void SetBatteryPresetFlag(uint8_t batt_type)
{
	pBatt->BattType = batt_type;
	
	if ((!pEEP->CCThreshold) || (pEEP->CCThreshold > DEF_CC_THRESHOLD))
		pEEP->CCThreshold = DEF_CC_THRESHOLD;
	
	switch (batt_type)
	{
		case BATT_TYPE_QLN:
			pEEP->SlowChrgMins = 30;
			if (pBatt->DesignCap <= 2800)
				pEEP->FastChrgMins = 300;
			else
				pEEP->FastChrgMins = 540;
			
			pEEP->ChrgupVolt = 8400;
			pEEP->RechrgThreshold = DEF_RECHRG_THRESHOLD;
			pEEP->SlowFastVolt = 5600;
			
			pEEP->FastChrgCurr = 1000;
			pEEP->AbnormalChrgCurr = pEEP->FastChrgCurr + 500;
			pEEP->ChrgTermCurr = 150;
			pEEP->NearlyDoneCurr = 200;
			
			pEEP->ColdOn = 0;
			pEEP->ColdOff = -3;
			pEEP->HotOff = 45;
			pEEP->HotOn = 42;
			pEEP->SlowChrgCurr = _200mA;

			pBatt->I2CState = READ_BATT_VOLT_QLN;
			break;
			
		case BATT_TYPE_ZQ3:	
			pBatt->I2CState = READ_BATT_VOLT_ZQ3;
			if (AUTH_SLV_ADDR_RLX == AUTH.BattSlvAddr)
				pBatt->BattType = BATT_TYPE_RLX0;
			break;
			
		case BATT_TYPE_RLX1:
			if (pEEP->FastChrgCurr > MAX_FAST_CHRG_CURR)
			{
				pEEP->FastChrgCurr = MAX_FAST_CHRG_CURR;
				pEEP->AbnormalChrgCurr = MAX_FAST_CHRG_CURR + 500;
			}
			pBatt->I2CState = READ_BATT_VOLT_QLN;
			break;

		case BATT_TYPE_RLX0:
			pEEP->SlowChrgMins = 120;
			pEEP->FastChrgMins = 540;
			
			pEEP->ChrgupVolt = 8400;
			pEEP->RechrgThreshold = DEF_RECHRG_THRESHOLD;
			pEEP->SlowFastVolt = 5600;
			
			pEEP->SlowChrgCurr = _200mA;
			pEEP->FastChrgCurr = MAX_FAST_CHRG_CURR;
			pEEP->AbnormalChrgCurr = pEEP->FastChrgCurr + 500;
			pEEP->NearlyDoneCurr = 340;
			pEEP->ChrgTermCurr = 180;
			
			pEEP->ColdOn = 1;
			pEEP->ColdOff = -1;
			pEEP->HotOff = 45;
			pEEP->HotOn = 43;
			
			pBatt->DesignCap = 3400;
			pBatt->I2CState = READ_BATT_VOLT_ZQ3;
			break;
			
		case BATT_TYPE_DEAD:
			if (!pEEP->FastChrgMins)			// If we come here for the first time
				pEEP->SlowChrgMins = DEAD_SLOW_TIMEOUT;
			pEEP->FastChrgMins = DEAD_FAST_TIMEOUT;
			
			pEEP->ChrgupVolt   = DEAD_CHRG_UP_VOLT;
			pEEP->SlowFastVolt = DEAD_SLOW_FAST_VOLT;
			
			pEEP->AbnormalChrgCurr = DEAD_ABNORMAL_CURR;
			pEEP->SlowChrgCurr     = DEAD_MAX_SLOW_CURR;
			pEEP->NearlyDoneCurr   = DEAD_NEARLY_DONE_CURR;
			pEEP->FastChrgCurr = DEAD_FAST_CHRG_CURR;
			pEEP->ChrgTermCurr = DEAD_DONE_CURR;
			
			pEEP->ColdOff = DEAD_COLD_OFF_TMP;
			pEEP->ColdOn  = DEAD_COLD_ON_TMP;
			pEEP->HotOn   = DEAD_HOT_ON_TMP;
			pEEP->HotOff  = DEAD_HOT_OFF_TMP;
			
			pBatt->DeadBattChrgCnt = 60;			// Do dead battery charging 1 minute
			pBatt->I2CState = SET_A2D_MUXES_DEAD;	// Set next i2c state
			return;
	}
	
	uint8_t batt_flags = (BATT_FLAGS << PORTR_OUT);
	uint8_t sreg = SREG;

	SREG = 0;									// Disable global interrupts
	SYS.BattDetect &= ~(batt_flags & 0x0F);		// Clear battery insert flag
	SYS.BattDetect |=  (batt_flags & 0xF0);		// Set battery present flag
	SREG = sreg;								// Enable global interrupts
	
	pBatt->MiscFlags = BATT_VERIFIED_bm;
	
	SYS.SkipSleep =TRUE;
}

//----------------------------------------------------------------------
//
// Prototype:	ShowFWVersion
//
// Description:	Routine to show Toaster/FUB Firmware version
//
// Parameters:	None
//
// Returns:		None
//
// Notes:
//----------------------------------------------------------------------
void ShowFWVersion(void)
{
	// ------ Show FW version "x.yz" in 3 steps by blinking LEDs with respective color -------
	// Step 1: Show major Version (x): Amber LED blinks x times. This must be a non zero value
	// Step 2: Show Minor Version (y): Green LED blinks y times. Jumps to Step 3 if y is zero
	// Step 3: Show Minor Version (z): Red LED blinks z times. Jumps to Step 1 if z is zero
	// Repeat above steps 1 to 3.
	if ((pBatt->PWMSettleCnt) && (!pBatt->WaitCnt))	// If time has come to jump to next LED blinking state
	{// Yes
		uint8_t led_pat = CHARGING_DISABLE;			// Select the default pattern as all LEDs off
		uint8_t duration = 0;						// Select the default duration as 1
		
		while (!duration)
		{
			if (++pBatt->PWMSettleCnt == REPEAT_SHOW_FW_VER)	// If this is the time to repeat the LED blinking
				pBatt->PWMSettleCnt = SHOW_MAJOR_FW_VER;		// Restart by showing major FW version
			
			switch (pBatt->PWMSettleCnt)
			{
				case SHOW_MAJOR_FW_VER:					// If this is the time to show major version
					duration = pBatt->FWMajorVer;			// Get the number of blinks
						led_pat = BLINK_MODERATE_AMBER;	// Select LED pattern as moderate Amber
					break;
				
				case SHOW_MINOR_VER_MSD:				// If this is the time to show minor version most significant digit (MSD)
					duration = pBatt->FWMinorVer / 10;		// Get the number of blinks
					if (duration)						// If minor version MSD is non zero
						led_pat = BLINK_MODERATE_GREEN;	// Select LED pattern as moderate green
					break;
				
				case SHOW_MINOR_VER_LSD:				// If this is the time to show minor version least significant digit (LSD)
					duration = pBatt->FWMinorVer % 10;		// Get the number of blinks
					if (duration)						// If minor version LSD is non zero
						led_pat = BLINK_MODERATE_RED;	// Select LED pattern as moderate red
					break;
				
				default:
					duration = 1;						// Keep an interval between color transitions
					led_pat = CHARGING_DISABLE;			// Don't show anything
			}
			if (!duration)								// If blinking duration is zero
				pBatt->PWMSettleCnt++;					// Jump to next state
		}
		SetChrgLEDPat(led_pat);							// Start blinking LEDs
		pBatt->WaitCnt = BLINK_TIME * duration-1;		// Obtain the wait for desired LED blinks
	}
}
