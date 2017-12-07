//-----------------------------------------------------------------------------
// FILENAME:  battcomm.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares functions to communicate with battery gas gauge periodically
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
#include "pwm.h"
#include "uart.h"
#include "debug.h"
#include "timers.h"
#include "macros.h"
#include "thermdefs.h"
#include "i2c_batt.h"
#include "battchrg.h"
#include "battcomm.h"
#include "sysparam.h"
#include "batt_defs.h"
#include "battfound.h"
#include "PP_BattDefs.h"
#include "jasper_ports.h"

// Global data structures
EEP_CHRG_DATA EEP[MAX_SLOTS];
EEP_CHRG_DATA *pEEP;
GAS_GAUGE_DATA Gauge;
BATT_DATA Batt[MAX_SLOTS];
BATT_DATA *pBatt;

//----------------------------------------------------------------------
//
// Prototype:	Do_I2C
//
// Description:	This is the regular housekeeping activities
//
// Parameters:	None
//
// Returns:		None
// 
// Notes: 
//----------------------------------------------------------------------
void Do_I2C(void)
{
	switch (pBatt->I2CState)
	{	
		// -------------QLN battery charging starts here ----------------------
		case READ_BATT_VOLT_QLN:									// Read battery voltage
			AccessBattChip(GG_SLV_ADDR_QLN, VOLTAGE, 2, 0, (uint8_t*)&Gauge.Volt);
			break;
			
		case READ_AVG_CURR_QLN:										// Rad average current
			AccessBattChip(GG_SLV_ADDR_QLN, AVG_CURRENT, 2, 0, (uint8_t*)&Gauge.AvgCurr);
			break;
			
		case READ_INST_CURR_QLN:									// Rad instantaneous current
			AccessBattChip(GG_SLV_ADDR_QLN, INST_CURRENT, 2, 0, (uint8_t*)&Gauge.GGInstCurr);
			break;
			
		case READ_BATT_TEMP_QLN:									// Read battery temperature
			AccessBattChip(GG_SLV_ADDR_QLN, TEMPERATURE, 2, 0, (uint8_t*)&Gauge.TmpKelvin);
			break;
			
		case READ_BATT_STATUS_QLN:									// Read battery status flags
			AccessBattChip(GG_SLV_ADDR_QLN, BATT_STATUS, 2, 0, Gauge.Status);
			break;
			
		case READ_CYCLE_CNT_QLN:									// Read cycle count
			if (!(pBatt->MiscFlags & FAKE_SOH_bm))					// If not in fake SOH mode
			{// Yes
				AccessBattChip(GG_SLV_ADDR_QLN, CYCLE_COUNT, 2, 0, (uint8_t*)&pBatt->ChrgCycleCount);
				break;
			}
			else
				pBatt->I2CState = READ_SOC_QLN;
			// No "break" here
			
		case READ_SOC_QLN:											// Read State of charge
			if (!(pBatt->MiscFlags & FAKE_SOC_bm))
			{
				AccessBattChip(GG_SLV_ADDR_QLN, REL_STATE_OF_CHRG, 1, 0, &pBatt->SOC);
				break;
			}
			else
				pBatt->I2CState = READ_SOH_QLN;
			// No "break" here
			
		case READ_SOH_QLN:											// Read state of health
			if (!(pBatt->MiscFlags & FAKE_SOH_bm))					// If not in fake SOH mode
			{// Yes
				AccessBattChip(GG_SLV_ADDR_QLN, STATE_OF_HEALTH, 1, 0, &pBatt->SOH);
				break;
			}
			else
				pBatt->I2CState = WAIT_A2D_SETUP_DELAY_QLN;
			// No "break" here
			
		case WAIT_A2D_SETUP_DELAY_QLN:
			if ((!pBatt->SOH) || (pBatt->SOH > 100))
				pBatt->SOH = 100-(pBatt->ChrgCycleCount/20);
			if (SetA2DMuxForCurrTmp())
				Wait_mSecs(A2D_READ_DELAY);
			break;
			
		case READ_A2D_CURR_TMP_QLN:
			if (ReadA2DCurrTmp(CHARGE_ENTRY_QLN))
				break;
			
		case CHARGE_ENTRY_QLN:
		{
			uint8_t curr_slot = PORTR_OUT;
			if (!(pBatt->MiscFlags & FAKE_TMP_bm))					// If we are not in fake temperature mode
			{
				int8_t tmp = (Gauge.TmpKelvin/10) - KELVIN_CONST;	// Calculate the battery temperature in Celsius
				// The following logic is in place just to prevent LED red blinking because of bad temperature reported by gas gauge of 4-Cell Extended batteries once they escaped from dead battery state 
				if (BAD_TMP_VALUE == tmp)										// If temperature reported by gas gauge is bad
				{
					if (INVALID_TMP == pBatt->Tmp)					// And last known temperature was also invalid
					{
						SYS.NextSlot = TRUE;						// Do not pass them to charger this time
						return;										// Just return and serve other slots
					}
					else
					{
						int8_t tmp_err;
						if (tmp >= pBatt->Tmp)
							tmp_err = tmp - pBatt->Tmp;
						else
							tmp_err = pBatt->Tmp - tmp;
							
						if (tmp_err > 3)						// If the error is two high
							tmp = pBatt->Tmp;					// Discard the new value and use the last known value
					}
				}
				pBatt->Tmp = tmp;								// Update the global data storage
			}
			
			Gauge.InstCurr = (A2D.Slot[curr_slot].ISenseConst * (int32_t)A2D.BattISense)/((uint16_t)16*A2D_RES);
			DoBattChrg(READ_BATT_VOLT_QLN);							// Call charger routine and set next I2C state
			break;
		}
		//-----------------------------------------------------------------------------------------------------
		
		// -------------ZQ3 battery charging starts here ----------------------
		case READ_BATT_VOLT_ZQ3:									// Read battery voltage
			AccessBattChip(GG_SLV_ADDR_ZQ3, BATT_VOLT_ZQ3, 2, 0, (uint8_t*)&Gauge.Volt);
			break;
			
		case READ_AVG_CURR_ZQ3:										// Rad average current
			AccessBattChip(GG_SLV_ADDR_ZQ3, AVG_CURR_ZQ3, 2, 0, (uint8_t*)&Gauge.AvgCurr);
			break;
			
		case READ_INST_CURR_ZQ3:									// Rad instantaneous current
			AccessBattChip(GG_SLV_ADDR_ZQ3, INST_CURR_ZQ3, 2, 0, (uint8_t*)&Gauge.GGInstCurr);
			break;
			
		case READ_BATT_TEMP_ZQ3:									// Read battery temperature
			AccessBattChip(GG_SLV_ADDR_ZQ3, BATT_TEMP_ZQ3, 2, 0, (uint8_t*)&Gauge.TmpKelvin);
			break;
			
		case READ_BATT_STATUS_ZQ3:									// Read battery status flags
			AccessBattChip(GG_SLV_ADDR_ZQ3, BATT_STATUS_ZQ3, 2, 0, Gauge.Status);
			break;
			
		case READ_CYCLE_CNT_ZQ3:									// Read cycle count
			AccessBattChip(GG_SLV_ADDR_ZQ3, CYCLE_CNT_ZQ3, 2, 0, (uint8_t*)&pBatt->ChrgCycleCount);
			break;
			
		case READ_SOC_ZQ3:											// Read state of charge
			if (!(pBatt->MiscFlags & FAKE_SOC_bm))
			{// Yes
				AccessBattChip(GG_SLV_ADDR_ZQ3, REL_SOC_ZQ3, 1, 0, &pBatt->SOC);
				break;
			}
			else
				pBatt->I2CState = READ_SOH_ZQ3;
			// No "break" here
			
		case READ_SOH_ZQ3:											// Read state of health
			if (!(pBatt->MiscFlags & FAKE_SOH_bm))					// If not in fake SOH mode
			{// Yes
				AccessBattChip(GG_SLV_ADDR_ZQ3, BATT_SOH_ZQ3, 1, 0, &pBatt->SOH);
				break;
			}
			else
				pBatt->I2CState = WAIT_A2D_SETUP_DELAY_ZQ3;
			// No "break" here
			
		case WAIT_A2D_SETUP_DELAY_ZQ3:
			if (SetA2DMuxForCurrTmp())
				Wait_mSecs(A2D_READ_DELAY);
			break;
			
		case READ_A2D_CURR_TMP_ZQ3:
			if (ReadA2DCurrTmp(CHARGE_ENTRY_ZQ3))
				break;
			
		case CHARGE_ENTRY_ZQ3:	
		{
			uint8_t curr_slot = PORTR_OUT;
			if (!(pBatt->MiscFlags & FAKE_TMP_bm))					// If we are not in fake temperature mode
				pBatt->Tmp = (Gauge.TmpKelvin/10) - KELVIN_CONST;	// Calculate the battery temperature in Celsius
			Gauge.InstCurr = (A2D.Slot[curr_slot].ISenseConst * (int32_t)A2D.BattISense)/((uint16_t)16*A2D_RES);
			DoBattChrg(READ_BATT_VOLT_ZQ3);							// Call charger routine and set next I2C state
			break;
		}
		//-----------------------------------------------------------------------------------------------------
	}
}

//-----------------------------------------------------------------------------
//
// Prototype:	DoDumbCalc
//
// Description:	Routine to calculate battery temperature, voltage and current from respective A2D readings
//
// Parameters:	None
//
// Returns:		None
//
// Notes:
//-----------------------------------------------------------------------------
void DoDumbCalc(void)
{
	uint8_t curr_slot = PORTR_OUT;
	Gauge.AvgCurr = (A2D.Slot[curr_slot].ISenseConst * (int32_t)A2D.BattISense)/((uint16_t)16*A2D_RES);
	Gauge.Volt = A2D.Slot[curr_slot].BattVSense - (Gauge.AvgCurr/100);

	if (!(pBatt->MiscFlags & FAKE_TMP_bm))			// If we are not in fake temperature mode
	{
		int8_t   Tmp[THERM_DATA_ENTRIES] = {TMP0, TMP1, TMP2, TMP3, TMP4, TMP5, TMP6, TMP7, TMP8};
		uint16_t Res[THERM_DATA_ENTRIES] = {RES0, RES1, RES2, RES3, RES4, RES5, RES6, RES7, RES8};
		uint16_t *pRes;
		uint32_t Rt, LastR=0;
		int8_t T, LastT = 0;

		Rt = ((uint32_t)EQU_RES*(A2D.BattTSense - (Gauge.AvgCurr/100)))/(K-A2D.BattTSense);	// Calculate the Thermistor resistance
		
		// Temperature linear approximation starts here
		pRes = Res;
		for (uint8_t cnt=0; cnt<THERM_DATA_ENTRIES; cnt++)
		{
			uint32_t R;
			uint8_t exponent;

			T = Tmp[cnt];							// Read the temperature from the table
			exponent = *pRes & 0xF;
			R = ((uint32_t)*pRes >> 4) << exponent;	// Calculate the resistance from the table

			if (Rt >= R)							// If Rthem > Rtable
			{
				if (cnt)							// If this is not the very first entry (which means LastR and LastT are valid)
				{//Yes
					uint32_t Delta_R = LastR - R;	// Calculate the divisor
					if (Delta_R)					// If the divisor is not zero (just to prevent divided by 0 runtime error)
					{
						uint32_t Delta_Rt = Rt - R;
						int16_t t = (int16_t)((32 * Delta_Rt * (T-LastT))/Delta_R);	// Calculate the 32 times offset
						if (Delta_Rt)				// If this is not a table entry value
							t += TMP_CORR_FACTOR;	// Add the temperature correction
						
						T -= t/32;					// First divide it by 32 and them subtract from the table temperature
					}
					else
						T = INVALID_TMP;			// We should not see this! (Something wrong with Thermistor data table)
				}
				break;								// We are done, just exit the loop
			}
			LastR = R;								// Remember last resistance and temperature from the table
			LastT = T;
			pRes++;									// Goto next entry
		}
		pBatt->Tmp = T;								// Stored in global storage
	}
}

//-----------------------------------------------------------------------------
//
// Prototype:	AccessBattChip
//
// Description:	Routine read/write data from/to battery chip
//
// Parameters:	slv_addr : Slave address of the battery chip
//				pointer  : Address of the I2C register to be accessed
//				read_cnt : No. of bytes to read
//				write_cnt: No. of bytes to write
// 				pbuf	 : pointer to read/write buffer
//
// Returns:		None
// 
// Notes: 	It is possible to do data writes followed by data reads. In such situations,
//			the length of pbuf must be equal or greater than sum of read_cnt and write_cnt
//-----------------------------------------------------------------------------
void AccessBattChip(uint8_t slv_addr, uint8_t pointer, uint8_t read_cnt, uint8_t write_cnt, uint8_t *pbuf)
{	
	if (I2CMASTER_IDLE == I2CBatt.State)	// If I2C hardware is ready
	{	
		I2CBatt.ChipAddr = slv_addr;		// Set the I2C slave address	
		I2CBatt.Pointer  = pointer;			// Set address of the I2C register map to be accessed
		I2CBatt.ReadCnt  = read_cnt;		// Set the read byte count
		I2CBatt.WriteCnt = write_cnt;		// Set the write byte count
		pI2CBattBuf = pbuf;					// Set the pointer to read/write buffer
		Start_I2CBatt();					// Start the I2C transaction
	}
}

//----------------------------------------------------------------------
//
// Prototype:	SIGNAL(TCC1_CCB_vect)
//
// Description:	Timer to keep 500us delay between I2C transactions 
//
// Parameters:	None
//
// Returns:		None
//
// Notes:
//----------------------------------------------------------------------
SIGNAL(TCC1_CCB_vect)
{
	TCC1_CTRLA = TC_CLKSEL_OFF_gc;
	TCC1_INTCTRLB = TC_CCBINTLVL_OFF_gc;
	SYS.Actives &= ~AUTH_TMR_bm;
	pBatt->I2CBusy = FALSE;	// Clear I2C busy flag
	pBatt->I2CState++;
}

//----------------------------------------------------------------------
//
// Prototype:	UpdateBattCommStatus
//
// Description:	Routine to update battery communication state
//
// Parameters:	state - ACK_bm / NACK_bm
//
// Returns:		None
// 
// Notes: 
//----------------------------------------------------------------------
void UpdateBattCommStatus(uint8_t state)
{
	pBatt->I2CBusy = FALSE;	// Clear I2C busy flag
	
	if (ACK_bm == state)	// The I2C transaction was successful
	{// Yes
		if (pBatt->I2CState == pBatt->CommErrI2CState)	// If the last I2C communication failure is not seen anymore
		{
			pBatt->CommRetryCnt = 0;					// Clear communication retry count
			pBatt->CommErrI2CState = 0;					// And the last comm error i2c state
		}
		//if ((AUTH_SLV_ADDR_ZQ3 == I2CBatt.ChipAddr) || (AUTH_SLV_ADDR_RLX == I2CBatt.ChipAddr) || (AUTH_SLV_ADDR_MLB == I2CBatt.ChipAddr))
		if (AUTH_SLV_ADDR_MLB == (I2CBatt.ChipAddr & (AUTH_SLV_ADDR_MLB | TWI_READ)))
		{
			if ((!AUTH.BattSlvAddr) && (I2CBatt.ChipAddr != AUTH_SLV_ADDR_MLB))
				AUTH.BattSlvAddr = AUTH.I2CSlvAddr;			// Remember the battery slave address 
			WaitAuthExecTime(AUTH.ExecTime);
		}
		else
		{
			// Keep 500us Delay between I2C transactions	
			pBatt->I2CBusy = TRUE;					// Set I2C busy flag
			TCC1_CNT = 0;
			TCC1_CCB = MICRO_SECS(500);
			TCC1_INTFLAGS = (TC1_CCAIF_bm | TC1_CCBIF_bm | TC1_OVFIF_bm);
			TCC1_INTCTRLB = TC_CCBINTLVL_MED_gc;
			TCC1_CTRLA = TC_CLKSEL_DIV4_gc;
			SYS.Actives |= AUTH_TMR_bm;
		}
		SYS.RetryCnt = 0;		// Reset retry count
	}
	else
	{
#ifdef _DEBUG_MSGS_
		//SendRS232('N');
		//SendHex(pBatt->I2CState);
#endif
		if (!I2CBatt.ChipAddr)
		{
			I2CBatt.ChipAddr = AUTH.I2CSlvAddr;
			WaitAuthExecTime(AUTH_WAKEUP_DELAY);
			return;
		}
		
		SYS.SkipSleep = TRUE;
		// It is possible to fail following I2C transactions if the respective auth chip was in
		// sleep mode by the time we send the I2C call. If so just proceed to next state
		if ((AUTH_INIT == pBatt->I2CState) || (AUTH_PUT_SLEEP_ZQ3 == pBatt->I2CState) || (AUTH_PUT_SLEEP_RLX == pBatt->I2CState))
		{
			pBatt->I2CState++;
			return;
		}
		
		if (++SYS.RetryCnt == MAX_I2C_RETRY_CNT)	// If all I2C retries are over
		{
			SYS.RetryCnt = 0;						// Reset retry count
			switch (pBatt->I2CState)
			{
				case READ_ID_FROM_FUB:				// If firmware update battery not found
					pBatt->I2CState = CHECK_DEAD_BATT_CHRG;		// Start dead battery charging followed by finding a PP battery
					break;
					
				case READ_DEV_NAME_QLN:							// If this is not a PP Battery
					pBatt->I2CState = AUTH_INIT;				// Try to find a PP+ battery
					break;
					
				case AUTH_WAKE:
					if (++SYS.BattDetCnt == MAX_BATT_DET_CNT)	// If no more attempts are left to find either type of battery
					{
						SYS.BattDetCnt = 0;						// Reset battery detect count
						pBatt->Fault = BATT_UNSUPPORTED;		// This battery is neither Jasper nor FUB. Report as unsupported battery
						//SendRS232('X');
						pBatt->I2CState = BATT_FAULT;			// Go to battery fault state
					}
					else
						{
							
						//SendRS232('Y');
						pBatt->I2CState = READ_ID_FROM_FUB;		// Try to find a firmware update battery
						}
					break;
					
				case READ_CC_THRESHOLD_QLN:						// If this command is not supported in gas gauge
					pBatt->I2CState = SET_BATT_PRESENT_FLAG;	// Just move to next state
					break;
					
				case READ_SOH_QLN:								// If SOH command is not supported in gas gauge
					pBatt->SOH = 0;								// Make it zero and move to next state
					pBatt->I2CState = WAIT_A2D_SETUP_DELAY_QLN;
					break;

				default:
					pBatt->CommErrI2CState = pBatt->I2CState;	// Remember the I2C comm err state
					//SetChrgLEDPat(CHARGING_DISABLE);		// Turn off LED indications
					pBatt->I2CState = COMM_FAULT;			// Assume this is due to battery removal
					pBatt->WaitCnt = REMOVAL_WAIT_CNT;		// Give 400ms to remove the battery from the bay
			}

#ifdef _DEBUG_MSGS_
			SendHex(I2CBatt.ChipAddr);
			SendHex(I2CBatt.Pointer);
#endif
		}
	}
#ifdef _DEBUG_MSGS_
	uint16_t tcc1_cnt = TCC1_CNT;
	if (tcc1_cnt > SYS.I2CGuardTime)
		SYS.I2CGuardTime = tcc1_cnt;
#endif
	TCC1_CNT = 0;					// Reset the I2C guard counter
}

//----------------------------------------------------------------------
//
// Prototype:	EncryptDecryptBattData
//
// Description:	Routine to encrypt/ decrypt FUB Data
//
// Parameters:	R16: Action to be performed (Zero:Decrypt, Non Zero:Encrypt)
//				X Reg: RAM address of data buffer
//				Y Reg: Address of the EEPROM (YL:0x00~0xFF, YH=0x00 or 0x02)
//				
// Returns:		None
// 
// Notes: This is a blocking call. 
//----------------------------------------------------------------------
void EncryptDecryptBattData(uint8_t type)
{
	register uint8_t action asm ("r16");// Use exclusively R16 for this variable
	uint8_t *pBuf;
	uint16_t addr;
	uint16_t faddr;

	//if (MCU.DEVID1 == 0x95)				// If this is a 32K Part
		faddr = AESEncryptDecrypt32K;	// Set  the flash address of the encryption routine in 32K part boot loader area
	//else
	//	faddr = AESEncryptDecrypt16K;	// Set  the flash address of the encryption routine in 16K part boot loader area

	action = type;
	pBuf = pI2CBattBuf;					// Set pointer to pre write buffer position
	if (DECRYPT_DATA == type)			// If this is a data decryption (i.e post read buffer position)
		pBuf -= FUB_BLOCK_SIZE;			// Get back to pre read buffer position

	addr = I2CBatt.Pointer;
	
	asm volatile (	"icall\n\t"			// Call the encryption routine in boot loader area
					:: 
					"r" (action),
					"z" (faddr),
					"y" (addr),
					"x" (pBuf)			// Store the (RAM) address in X register
					);	
}
