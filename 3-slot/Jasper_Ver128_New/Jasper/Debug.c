//-----------------------------------------------------------------------------
// FILENAME:  debug.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares functions handling debug/bootloader commands and debug messages
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
// %End 
//-----------------------------------------------------------------------------
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stddef.h>
#include "a2d.h"
#include "pwm.h"
#include "led.h"
#include "uart.h"
#include "debug.h"
#include "macros.h"
#include "eeprom.h"
#include "version.h"
#include "i2c_batt.h"
#include "sysparam.h"
#include "BattComm.h"
#include "battchrg.h"
#include "batt_defs.h"
#include "PP_BattDefs.h"
#include "jasper_ports.h"

// Variables used for debugging
uint8_t Cmd[WR_CMD_SIZE];
uint8_t ByteCnt, CmdSize, CmdType, CmdData, Cksum;
extern void __bss_end;
DEBUG_DATA DD[MAX_SLOTS];

//------------------------------------------------------------------------------------
//
// Prototype: 	Init_Debug
//
// Description:	Initialize variables related to boot loader commands, debug messages
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void Init_Debug(void)
{
	ByteCnt = 0;
	CmdType = 0;
	CmdData = 0;
	memset(Cmd, 0, sizeof(Cmd));
	SYS.DebugCnt = NO_DEBUG;
	CmdSize = WR_CMD_SIZE;

	// Enable RXD pin change interrupt just to catch very first Jasper command send through UART
	// Enable RXD pin change interrupt just to catch very first Falcon command send through UART
	CLEAR_PENDING_RX_PIN_CHANGE_INTS;
	ENABLE_RX_PIN_CHANGE_INT;
	//SYS.Actives |= UART_RX_bm;		// Add Uart receiver to active peripherals list

	memset(DD, 0, sizeof(DD));
	for (uint8_t slot=0; slot < MAX_SLOTS; slot++)
		DD[slot].Tmp = INVALID_TMP;
}

//----------------------------------------------------------------------
//
// Prototype: 	DoBLCommands
//
// Description:	Routine execute bootloader/FUB commands from debug serial port
//
// Parameters:	None
//
// Returns:		None
//
// Notes:
//----------------------------------------------------------------------
void DoBLCommands(void)
{
	uint8_t cksum=0, cnt, cmd;
	uint8_t *buf;
	
	buf = &FRAME.BytePos;
	cmd = FRAME.Cmd;
	FRAME.BytePos = SYNC_BYTE;
	FRAME.Cmd = CMD_SUCCESS;
	FRAME.Length = 1;
	
	switch (cmd)
	{
		case ENTER_BOOTLOADER:
			SYS.ModeReg = ENTER_BL_bm;
			break;
		
		case SEND_BATT_CHRG_DATA:
			if (LED.ShowYourSelf)
				SetShowYourSelf(FALSE);
				
			FRAME.Length  = 7 + sizeof(DD);
			FRAME.Data[0] = SYS.ModeReg;
			FRAME.Data[1] = JASPER_3SLOT_ID;
			FRAME.Data[2] = VERSION_MAJOR;
			FRAME.Data[3] = VERSION_MINOR;
			FRAME.Data[4] = pgm_read_byte(0x3FFF);
			FRAME.Data[5] = MAX_SLOTS+1;//SYS.BestSlot1;
			memcpy(&FRAME.Data[6], (uint8_t*)DD, sizeof(DD));
			break;
			
		case SET_BEST_BATT_SLOT:
			SYS.BestSlot = FRAME.Data[0];
			break;
			
		case SHOW_YOURSELF:
			SetShowYourSelf(FRAME.Data[0]);
			break;	
			
		case GET_TOASTER_INFO:
			FRAME.Length = 3 + strlen(JASPER_NAME);
			FRAME.Data[0] = JASPER_3SLOT_ID;
			FRAME.Data[1] = MAX_SLOTS;
			memcpy(&FRAME.Data[2], JASPER_NAME, strlen(JASPER_NAME));
			break;
			
			
		case COPY_FUB_DATA:
			// This command enables us to program FUBs from Slot1
			if ((Batt[SLOT1].I2CState == SIMPLY_WAIT) && (FRAME.DataCnt == (FUB_PAGE_SIZE + 2)))
			{
				// [Byte 0: Address MSB]
				// [Byte 1: Address LSB]
				// [Bytes 2:65: Encrypted Data]
				uint16_t mem_addr = (((uint16_t)FRAME.Data[0] << 8) | FRAME.Data[1]);
				if (!mem_addr)
					SYS.FUBMemAddr = 0;
					
				if (mem_addr == SYS.FUBMemAddr)
				{
					if (!mem_addr)
					{
						// It is possible to remove the FUB during programming is on process.
						// Therefore it is a good idea to invalidate existing FUB firmware first.
						Batt[SLOT1].PWMSettleCnt = STOP_SHOW_FW_VER;	// We are about to start FUB programming. Stop Amber Blink
						memset(SYS.FUBDataBuf, 0x00, (FUB_PAGE_SIZE+2));// Write zeros to last block where FW version and toaster type is stored
						pI2CBattBuf = &SYS.FUBDataBuf[50];				// Set the pointer to encrypt last 16 bytes
						I2CBatt.Pointer = 0;							// Select the encryption key
						EncryptDecryptBattData(ENCRYPT_DATA);			// Encrypt last 16 bytes before write
						SYS.FUBDataBuf[0] = ID_BLOCK_ADDR_HI;			// Select the address of the last FUB page
						SYS.FUBDataBuf[1] = 0xC0;
						SYS.FUBMemAddr = -FUB_PAGE_SIZE;
					}
					else
						memcpy(SYS.FUBDataBuf, FRAME.Data, (FUB_PAGE_SIZE+2));	// Get a copy of FUB data
						
					Batt[SLOT1].I2CState = WRITE_FUB_PAGE;
					SYS.ScanSlots = TRUE;	// It's time to scan battery slots
					SYS.SkipSleep = TRUE;	// Just in case skip the sleep
				}
				else
				{
					LED.Pat.Slot[SLOT1].Grn = LED_OFF_PAT;
					LED.Pat.Slot[SLOT1].Red = FAST_BLINK_PAT;		// One or more data packets are missing. Show error status
				}
			}
			break;
			
		
		default:
			FRAME.Cmd = CMD_INVALID;
	}
	
	if (cmd != COPY_FUB_DATA)
	{
		SendRS232(ACK);
		for (cnt = 0; cnt < (FRAME.Length+3); cnt++)
		{
			uint8_t data = *buf;
			SendRS232(data);
			cksum += data;
			buf++;
		}
		SendRS232(-cksum);
	}
}

//------------------------------------------------------------------------------------
//
// Prototype: 	void DoDebugCmds
//
// Description:	Routine to handle debug commands and boot loader commands
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void DoDebugCmds(uint8_t recv_byte)
{	
	uint8_t *p_soc = 0;
	uint8_t offset = 0;

	if (!ByteCnt)						// If this not a boot loader command
	{
		BATT_DATA *pbatt;
		pbatt = &Batt[SYS.DebugSlot];	// Set pointer to Batt structure in current debug slot

		switch (recv_byte)
		{	
			case 'R': CmdSize = RD_CMD_SIZE;	// If boot loader commands?
			case 'W':
				SYS.DebugCnt = NO_DEBUG;		// Stop debug messages
				Cmd[ByteCnt++] = recv_byte; 	// Add to command buffer
				break;
				
			case 'D': SYS.DebugSlot = MAX_SLOTS;
			case 'd':							// If 'd' is pressed?
				if (NO_DEBUG == SYS.DebugCnt)	// Toggle debug messages
					SYS.DebugCnt = 0;
				else
					SYS.DebugCnt = NO_DEBUG;
				break;	
				
			case 'f':					// If 'f' is pressed?
				PrintFreeRAM();			// Print Free RAM
				break;

			case 'v':					// If 'v' is pressed?
				PrintVersionString();	// Print version
				break;

			case 'p':					// If 'p' is pressed?
				PrintPortValues();		// Print port values
				break;

			case 'o':					// If 'o' is pressed?
				if (PORTB_INTCTRL)
					HandleOverCurrInt();
				break;
				
			case 'i':
				PrintBattEEPData();
				break;
				
			case 'j':
				PrintBattEEPData2();
				break;
				
			case 'k':
				{											// If 'k' is pressed?
					uint8_t slots = MAX_SLOTS;
					pbatt = Batt;						// Apply force calibration to all slots	

					Print_P(PSTR("Slot I2CStat I2CBusy ChrgStat"));
					while (slots--)
					{
						Print_P(PSTR("\r\n"));
						SendHex(2-slots);
						SendHex(pbatt->I2CState);
						SendHex(pbatt->I2CBusy);
						SendHex(pbatt->ChrgState);
						pbatt++;
					}
					Print_P(PSTR("\r\nActives:"));
					SendHex(SYS.Actives);
					SendHex(SYS.BattDetect);
					SendHex(TCC1_CTRLA);
					SendHex(TCC1_INTCTRLA);
					SendHex(TCC1_INTCTRLB);
				}
				
			case TAB_KEY:							// if TAB key is pressed
				if (++SYS.DebugSlot > MAX_SLOTS)	// Move to next debug slot
					SYS.DebugSlot = SLOT0;
				SYS.DebugCnt = 1;
				break;				
				
			// ---------- The following two commands let us test SLOW CURR FAULT/ FAST CURR FAULT conditions --------------
			// ---------- by manually adjusting the charging current at slow charge/ fast charge states -------------------				
			case '-':										// If '-' or '+' is pressed?
			case '+':
				if (SYS.DebugSlot < MAX_SLOTS)				// If we are in single slot debugging mode
				{
					if ((pbatt->ChrgState >= START_SLOW_CHRG) && (pbatt->ChrgState <= CHRG_NEARLY_DONE))	// If battery is charging
					{
						volatile uint16_t *pwmi_reg;
						pwmi_reg = PWMIReg(SYS.DebugSlot);
						int16_t pwmi = *pwmi_reg;			// Get the current PWM I value
						pwmi += (0x2C - recv_byte);			// Increment or decrement it by one

						if (pwmi > PWM_I_MAX)				// If exceeds the maximum limit
							pwmi = PWM_I_MAX;				// Keep at maximum

						if (pwmi < 0)						// If goes minus
							pwmi = 0;						// Keep at zero

						*pwmi_reg = pwmi;					// Set the new PWM I value
							
						pbatt->MiscFlags |= MANUAL_PWMI_bm;	// Set manual PWMI flag so that auto adjusting will be disabled

						if (SYS.DebugCnt)
							SYS.DebugCnt = 1;				// Because we need immediate update of PWMI
					}
				}					
				break;
					
			// ---------- The following two commands let us test SLOW CURR FAULT/ FAST CURR FAULT conditions --------------
			// ---------- by manually adjusting the charging current at slow charge/ fast charge states -------------------	
				/*	
				if (SYS.DebugSlot < MAX_SLOTS)				// If we are in single slot debugging mode
				{
					if ((pbatt->ChrgState >= START_SLOW_CHRG) && (pbatt->ChrgState <= CHRG_NEARLY_DONE))	// If battery is charging
					{
						volatile uint16_t *pwmv_reg;
						pwmv_reg = PWMVReg(SYS.DebugSlot);
						int16_t pwmv = *pwmv_reg;			// Get the current PWM I value
						pwmv += (0x2C - recv_byte);			// Increment or decrement it by one

						if (pwmv > PWM_V_MAX)				// If exceeds the maximum limit
							pwmv = PWM_V_MAX;				// Keep at maximum

						if (pwmv < 0)						// If goes minus
							pwmv = 0;						// Keep at zero

						*pwmv_reg = pwmv;					// Set the new PWM I value
						
						//pbatt->MiscFlags |= MANUAL_PWMI_bm;	// Set manual PWMI flag so that auto adjusting will be disabled

						//if (SYS.DebugCnt)
						SYS.DebugCnt = 1;				// Because we need immediate update of PWMI
					}
				}
				break;
				*/
				
			// ---------- The following two commands let us test battery charging at extreme ------------------------------
			// ---------- temperatures without actually doing it inside a temperature chamber -----------------------------
			case '<':										// If '<' or '>' is pressed? (Don't forget to press "Shift" key)
			case '>':
				offset = 0x3D;			// 0x3D = ('<'+'>')/2
				break;
				
			// ---------- Without actually waiting for a battery to become unhealthy (TI), the following ------------------
			// ---------- two commands let us test various LED blinking patterns under unhealthy condition ----------------
			case '[':										// If '[' or ']' is pressed?
			case ']':
				offset = 0x5C;			// 0x5C = ('['+']')/2
				break;
					
			// ---------- The following two commands is to adjust battery capacity -------------------------
			// ---------- It helps validating the Best of 4 algorithm  -------------------------------------
			case '{':										// If '{' or '}' is pressed?
			case '}':
				p_soc = &pbatt->SOC;
				offset = 0x7C;
				break;
					
					
			case 'z':
			case 'x':
				offset = 0x79;
				break;
		}
			
		if ((offset) && (SYS.DebugSlot < MAX_SLOTS))			// If we are in single slot debuging mode
		{
			if ((pbatt->ChrgState) && (pbatt->ChrgState <= BAD_TMP))	// If battery is charging
			{
				int8_t delta = (recv_byte - offset);
				switch (offset)
				{
					case 0x3D:
						pbatt->Tmp += delta;				// This will roll over (-128 <-> 127)
						pbatt->MiscFlags |= FAKE_TMP_bm;	// Start showing fake battery temperature
						break;
					
					case 0x5C:
						if (BATT_TYPE_QLN == pbatt->BattType)
							pbatt->ChrgCycleCount += delta;
						else
							pbatt->SOH += delta;			// This will roll over (0 <-> 255)
						pbatt->MiscFlags |= FAKE_SOH_bm;// Start showing fake battery health
						break;
						
					case 0x7C:
						pbatt->MiscFlags |= FAKE_SOC_bm;// Start showing fake battery SOC
					case 0x69:
					{
						int8_t soc = *p_soc;
						soc += delta;					// Increment or decrement
						if (soc < 0)					// if goes negative
							soc = 0;					// Make it 0
						if (soc > 100)					// If goes above 100
							soc = 100;					// Make to 100
						*p_soc = soc;					// Store the modified SOC
					}
					break;
						
					case 0x79:
					{
						int16_t design_cap = pbatt->DesignCap;	// Get the current design capacity
						design_cap -= 10*delta;					// Increment or decrement by 10
						if (design_cap < 10)					// if goes bellow 10
							design_cap = 10;					// Make it 10
						if (design_cap > 9990)					// If goes above 9990
							design_cap = 9990;					// Make to 9990
						pbatt->DesignCap = design_cap;			// Store the modified design capacity
					}
					break;
				}	
			}
			if (SYS.DebugCnt)
				SYS.DebugCnt = 1;
		}
	}		
	else	// Must be 'R' or 'W' command
	{
		Cmd[ByteCnt++] = recv_byte;	// Keep adding to command buffer
			
		if (ByteCnt >= CmdSize)		// If complete command receives
		{
			uint8_t ptr_hi, ptr_lo, ptr;

			ByteCnt = 0;			// Reset command byte counter
			ptr_hi = AsciiToHex(Cmd[1]);// Get the I2C Pointer
			ptr_lo = AsciiToHex(Cmd[2]);

			if ((ptr_hi < 0x10) && (ptr_lo < 0x10))	// Validate ptr
				ptr = (ptr_hi<<4) | ptr_lo;
			else
			{
				SendRS232(NACK);	// Send NACK
				return;				// Done
			}
			
			if (Cmd[0] == 'W')		// Write I2C register command
			{
				uint8_t data_hi, data_lo;
				data_hi = AsciiToHex(Cmd[3]);
				data_lo = AsciiToHex(Cmd[4]);
				if ((data_hi < 0x10) && (data_lo < 0x10))	// Validate data
				{
					uint8_t data = (data_hi<<4) | data_lo;
					SYS.ModeReg = (data & ENTER_BL_bm);
					SendRS232(ACK);
				}
				else
					SendRS232(NACK);		// Invalid data, Send NACK
						
			}
			else	// Read from I2C register command	
			{
				uint8_t data;
				data = HandleRead(ptr);		// Read data from relevant I2C Reg
				Cmd[0] = ACK;				// Assemble response packet
				Cmd[1] = HexToAscii(data>>4);
				Cmd[2] = HexToAscii(data&0xF);
				Cmd[3] = 0;
				Print(Cmd);					// Send the response followed by ACK
			}
				
			CmdSize = WR_CMD_SIZE;
			memset(Cmd, 0, sizeof(Cmd));	// Init command buffer
		}
	}
}

//------------------------------------------------------------------------------------
//
// Prototype: 	SetupBattChrgData
//
// Description:	Routine to Setup battery charging information for BT Smart interface
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void SetupBattChrgData(void)
{
	uint8_t curr_slot = PORTR_OUT;
	DEBUG_DATA *pDD;
	pDD = DD+curr_slot;

	if (TRUE == SYS.NextSlot)			// If there is no battery in the slot or charger fault
	{
		memset(&Gauge, 0, sizeof(GAS_GAUGE_DATA));	// Clear the structure first
	
		if (!(pBatt->MiscFlags & FAKE_TMP_bm))		// If this is not fake temperature
			pBatt->Tmp = INVALID_TMP;				// Make the battery temperature invalid
			
		if (!(pBatt->MiscFlags & FAKE_SOH_bm))		// If this is not fake SOH
			pBatt->SOH = 0;							// Clear battery SOH

		pBatt->ChrgState = 0;						// Clear charger state		
		pBatt->ChrgTimeSecs = 0;					// Clear all timers		
		pBatt->SOC = 0x7F;
	}
	
	pDD->Volt = Gauge.Volt;
	pDD->Curr = Gauge.GGInstCurr;	
	pDD->ChrgCycleCnt = pBatt->ChrgCycleCount;
	pDD->Tmp = pBatt->Tmp;
	pDD->SOC = pBatt->SOC;
	
	pDD->SOH = pBatt->SOH;
	if (pBatt->Fault)
		pDD->ChrgState = 0x80 | pBatt->Fault;
	else
		pDD->ChrgState = pBatt->ChrgState;
	pDD->RatedCap = pBatt->DesignCap;
	pDD->HealthThreshold = 80;
}

//------------------------------------------------------------------------------------
//
// Prototype: 	PrintBattChrgInfo
//
// Description:	Print battery charging information in all or selected slot
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void PrintBattChrgInfo(void)
{
	uint8_t curr_slot = PORTR_OUT;

	if (SYS.DebugSlot < MAX_SLOTS)		// If single slot debugging mode
	{
		if (SYS.DebugSlot != curr_slot)	// If this is not the selected debugging slot 
			return;						// Just return
		SYS.DebugCnt = ONE_SEC;			// Reset debug count one second
	}
	else								// Debugging all 3 slots
	{	//		_________		_________		_________		_________		_________		_________		_________		_________		_________		_________
		//		| | | | |		| | | | |		| | | | |		| | | | |		| | | | |		| | | | |		| | | | |		| | | | |		| | | | |		| | | | |
		// 		|0|1|2|3|		|0|1|2|3|		|0|1|2|3|		|0|1|2|3|		|0|1|2|3|		|0|1|2|3|		|0|1|2|3|		|0|1|2|3|		|0|1|2|3|		|0|1|2|3|	
		// _____|_|_|_|_|_______|_|_|_|_|_______|_|_|_|_|_______|_|_|_|_|_______|_|_|_|_|_______|_|_|_|_|_______|_|_|_|_|_______|_|_|_|_|_______|_|_|_|_|_______|_|_|_|_|_
		//	 900			   0 /|\		 100			 200      		 300	/|\		 400	    	 500			 600	 /|\   	 700			 800  Time(ms)
		// |---------|----------| |													 |				  								  |	
		// |Time (ms)|Debug Slot| |													 |				  								  |	
		// |---------|----------| |													 |				  								  |	
		// | 	  0  |	Slot 0 --/													 |				  								  |	
		// |	200  |  Slot 1 -----------------------------------------------------/								                  |									
		// |	400  |  Slot 2 ------------------------------------------------------------------------------------------------------/									
		// |---------|----------|
		
		//if (SYS.OneSecCnt != 2*curr_slot)// If this is not the time to send debug messages of this slot
		if (curr_slot != (SYS.OneSecCnt/3))// If this is not the time to send debug messages of this slot
			return;						// Just return
		SYS.DebugCnt = 3;				// Reset debug count to 300mSecs
	}
		
	// Refer "DoBattChrg" routine in "BattChrg.c" to figure out when "SYS.NextSlot != TRUE"
	
	if (TRUE == SYS.NextSlot)			// If there is no battery in the slot or charger fault
	{
		//memset(&Gauge, 0, sizeof(GAS_GAUGE_DATA));	// Clear the structure first
	
		//if (!(pBatt->MiscFlags & FAKE_TMP_bm))		// If this is not fake temperature
		//	pBatt->Tmp = INVALID_TMP;				// Make the battery temperature invalid
		
		pBatt->SOC = 0;								// Clear state of charge
		//pBatt->ChrgState = 0;						// Clear charger state
		//pBatt->ChrgTimeSecs = 0;					// Clear all timers
	}
		
	Print_P(PSTR(SAVE_CURSOR_CMD MOVE_CURSOR_03_01_MSG SLOT_MSG));			// Save current cursor position and move to 03,01 position
	//----------- Row No:1 ---------------------------------
	
	SendDig(curr_slot, 1);							// Print the charger slot which debug messages are coming from
	
	Print_P(PSTR(TAB_KEY_MSG TAB_MSG VOLT_MSG));	// Print battery voltage
	SendDig(Gauge.Volt, 6);

	Print_P(PSTR(VOLTS_MSG TAB_MSG CC_COUNT_MSG));	// Print battery cycle count
	SendDig(pBatt->ChrgCycleCount, 3);
	
	Print_P(PSTR(TAB_MSG BATT_TYPE_MSG));			// Print battery type
	
	switch (pBatt->BattType)
	{
		case BATT_TYPE_NONE:	Print_P(PSTR(BATT_NONE_MSG)); break;
		case BATT_TYPE_DEAD:	Print_P(PSTR(BATT_DEAD_MSG)); break;
		case BATT_TYPE_QLN:		Print_P(PSTR(BATT_QLN_MSG)); break;
		case BATT_TYPE_ZQ3:		Print_P(PSTR(BATT_ZQ3_MSG)); break;
		case BATT_TYPE_RLX0:	Print_P(PSTR(BATT_RLX0_MSG)); break;
		case BATT_TYPE_RLX1:	Print_P(PSTR(BATT_RLX1_MSG)); break;
		case BATT_TYPE_FUB:		Print_P(PSTR(BATT_FUB_MSG)); break;
	}

	//----------- Row No:2 --------------------------------
	
	Print_P(PSTR(CRLF_MSG CHG_DIS_MSG));	// Print fuse charger state
	if (PORTA_OUT & (CHG_DIS0_bm << curr_slot))
		Print_P(PSTR(DISABLE_MSG TAB_MSG CURR_MSG));
	else
		Print_P(PSTR(ENABLE_MSG TAB_MSG CURR_MSG));

	SendDig(Gauge.AvgCurr, 6);				// Print battery average current
	
	Print_P(PSTR(AMPS_MSG TAB_MSG CC_TRSHLD_MSG));	// Print charge current accumulator update time
	SendDig(pEEP->CCThreshold, 3);	
	
	Print_P(PSTR(TAB_MSG INST_CURR_MSG));
	SendDig(Gauge.GGInstCurr, 6);			// Print battery instantaneous current
	
	//----------- Row No:3 ------------------------------
	
	Print_P(PSTR(AMPS_MSG CRLF_MSG CMOD_MSG));
	if (PORTA_OUT & (CMOD0_bm << curr_slot))
		Print_P(PSTR(HIGH_MSG));
	else
		Print_P(PSTR(LOW_MSG));
	
	if (pBatt->MiscFlags & FAKE_TMP_bm)		// Print battery temperature
		Print_P(PSTR(TAB_MSG FAKE_TMP_MSG));
	else
		Print_P(PSTR(TAB_MSG TEMP_MSG));
	SendDig(pBatt->Tmp, 3);

	Print_P(PSTR(CELCIUS_MSG TAB_MSG CHRG_STATE_MSG));	// Print the dyna block to be updated next time
	SendHex(pBatt->ChrgState);
	
	Print_P(PSTR(TAB_MSG DESIGN_CAP_MSG));	// Print Design Cap
	SendDig(pBatt->DesignCap, 4);
	
	//Print_P(PSTR(TAB_MSG));
	//SendHex(ACA_STATUS & AC_AC0STATE_bm);
	//----------- Row No:4 --------------------------------

	Print_P(PSTR(CRLF_MSG PWM_V_MSG));		// Print voltage PWM
	SendDig(GetPWMV(curr_slot), 4);
	
	Print_P(PSTR(DIV4096_MSG TAB_MSG SOC_MSG));
	SendDig(pBatt->SOC, 3);
	
	Print_P(PSTR(PERCENT_MSG TAB_MSG I2C_STATE_MSG));	// Print I2C state
	SendHex(pBatt->I2CState);
	
	Print_P(PSTR(TAB_MSG DBC_CNT_MSG));
	SendDig(pBatt->DeadBattChrgCnt, 2);
	
	//----------- Row No:5 --------------------------------

	Print_P(PSTR(CRLF_MSG PWM_I_MSG));		// Print current PWM
	SendDig(GetPWMI(curr_slot), 4);

	SendRS232('/');
	SendDig(PWM_I_MAX + 1, 4);

	if (pBatt->MiscFlags & FAKE_SOH_bm)				// Print battery health
		Print_P(PSTR(TAB_MSG FAKE_SOH_MSG));
	else
		Print_P(PSTR(TAB_MSG SOH_MSG));
	SendDig(pBatt->SOH, 3);
	
	Print_P(PSTR(TAB_MSG CHRG_ERR_MSG));	// Print charger fault
	SendHex(pBatt->Fault);
	
	Print_P(PSTR(TAB_MSG ERR_I2C_MSG));
	SendHex(pBatt->CommErrI2CState);

	// ---------- Row No:6 --------------------------------
	
	Print_P(PSTR(CRLF_MSG V_SENSE_MSG));	// Print batt voltage sense from A2D
	SendDig(A2D.Slot[curr_slot].BattVSense, 6);
	
	Print_P(PSTR(TAB_MSG BATT_STATUS_MSG));
	SendHex(Gauge.Status[HIGH_BYTE]);
	SendHex(Gauge.Status[LOW_BYTE]);
	
	Print_P(PSTR(TAB_MSG I_SENSE_MSG));	// Print current sense by A2D
	SendDig(Gauge.InstCurr, 6);

	Print_P(PSTR(TAB_MSG CHRG_TIME_MSG));	// Print charging time
	SendDig(((pBatt->ChrgTimeSecs + 59)/60), 3);

	//-------------------------------------------
	
	//----------------------------------------------
	Print_P(PSTR(MINS_MSG RESTORE_CURSOR_CMD));// Restore cursor	
}

//------------------------------------------------------------------------------------
//
// Prototype: 	PrintVersionString
//
// Description:	Clears the hyper term screen and prints Toaster name, version and checksum
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void PrintVersionString(void)
{
	uint8_t cksum = pgm_read_byte(APP_SECTION_END);	// Read the checksum byte
	
	Print_P(PSTR(CLEAR_SCREEN_CMD MSG JASPER_NAME_MSG));// Clear the screen and Print the name of the project
	
	SendDig(VERSION_MAJOR, 1);		// Print firmware version number
	SendRS232('.');
	SendDig(VERSION_MINOR, 2);

	Print_P(PSTR(CHECKSUM_MSG));	// Print firmware checksum
	SendHex(cksum);
	
	Print_P(PSTR(MOVE_CURSOR_10_00_MSG));	// Print cursor	

	if (SYS.DebugCnt != NO_DEBUG)
		SYS.DebugCnt = 1;
}

//------------------------------------------------------------------------------------
//
// Prototype: 	Init_FreeRAM
//
// Description:	Routine to fill free RAM with known data, here we use 0xAA
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:		This function should call at the very beginning, before the interrupts are enabled
//				"&__bss_end" returns the starting address of Free RAM
//				For more info visit following website
//				"http://forum.pololu.com/viewtopic.php?f=10&t=989&view=unread#p4218" 
//------------------------------------------------------------------------------------
void Init_FreeRAM(void)
{	
	register uint8_t *ptr;
	ptr = &__bss_end;	
	while ((uint16_t)ptr < SP)	// Loop until current Stack Pointer is met ("ptr <= SP" is also possible, but too risky!)
	{
		*ptr = 0xAA;	// Fill the Free RAM with known data		
		ptr++;			// Move to the next RAM address
	}
}

//------------------------------------------------------------------------------------
//
// Prototype: 	PrintFreeRAM
//
// Description:	Print the amount of Free RAM in bytes
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:		"&__bss_end" returns the starting address of Free RAM
//				For more info visit following website
//				"http://forum.pololu.com/viewtopic.php?f=10&t=989&view=unread#p4218" 
//------------------------------------------------------------------------------------
void PrintFreeRAM(void)
{
	register uint8_t *ptr;
	register uint16_t FreeRAM = 0;
	ptr = &__bss_end;
	while ((uint16_t)ptr <= INTERNAL_SRAM_END)	// Scan the entire SRAM
	{
		if (0xAA == *ptr)			// If filled data byte 0xAA is found
			FreeRAM++;				// Increment the Free RAM counter
		else
			break;
		ptr++;						// Move to the next RAM address
	}

	Print_P(PSTR(FREE_RAM_MSG));	// Print "Free RAM:" message
	SendDig(FreeRAM, 4);			// Print Free RAM in "DDDD" (4 digits) format
	Print_P(PSTR(CMD_PROMPT_MSG));
}
//------------------------------------------------------------------------------------
//
// Prototype: 	PrintPortValues
//
// Description:	Print the current values of PORTA, PORTB, PORTC, PORTD and PORTE
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void PrintPortValues(void)
{
	Print_P(PSTR(PORTS_MSG));
	volatile uint8_t* pIn;
	
	// Print port values starting from PORTA to PORTR
	pIn = &PORTA_IN;
	for (uint8_t a=0; a<5; a++)
	{
		SendHex(*pIn);
		pIn += 0x20;
	}
	
	SendHex(PORTR_IN);
	Print_P(PSTR(CMD_PROMPT_MSG));
}

//------------------------------------------------------------------------------------
//
// Prototype: 	HandleRead
//
// Description:	Routine to read from Jasper register
//
// Parameters:	reg  - Address of the Jasper register
//
// Returns:		Value of the Jasper register pointed by reg
//
// Notes:
//------------------------------------------------------------------------------------
uint8_t HandleRead(uint8_t reg)
{
	uint8_t data = 0;

	switch (reg)
	{
		case MODE_REG:
			data = SYS.ModeReg;
			break;

		case FIRM_VER_MAJOR:	// Firmware version, major then minor
			data = VERSION_MAJOR;
			break;

		case FIRM_VER_MINOR:
			data = VERSION_MINOR;
			break;

		case TOASTER_TYPE:
			data = JASPER_3SLOT_ID;
			break;
	}
	return data;
}

//----------------------------------------------------------------------
//
// Prototype:	SIGNAL (PORTC_INT1_vect)
//
// Description:	UART command receive interrupt
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes: 
//
//----------------------------------------------------------------------
SIGNAL (PORTC_INT1_vect)
{
	DISABLE_RX_PIN_CHANGE_INT;		// Disable RX pin change interrupts
	SLEEP_CTRL = SLEEP_SEN_bm;		// Enable Sleep at idle mode
	SYS.Actives |= UART_RX_bm;		// Add Uart receiver to active peripherals list
}

//----------------------------------------------------------------------
//
// Prototype:	PrintBattEEPData
//
// Description:	Routine to print charger control parameters
//
// Parameters:	None
//
// Returns:		None
//
// Notes:
//
//----------------------------------------------------------------------
void PrintBattEEPData(void)
{
	if (SYS.DebugSlot < MAX_SLOTS)
	{
		EEP_CHRG_DATA *peep;
		peep = EEP + SYS.DebugSlot;
	
		Print_P(PSTR("Slow Chrg Mins:"));
		SendDig(peep->SlowChrgMins, 3);
		Print_P(PSTR("\r\nFast Chrg Mins:\t"));
		SendDig(peep->FastChrgMins, 3);
	
		Print_P(PSTR("\r\nChrg Up Volt:\t"));
		SendDig(peep->ChrgupVolt, 6);
		Print_P(PSTR("\r\nSlow Fast Volt:\t"));
		SendDig(peep->SlowFastVolt, 6);
	
		Print_P(PSTR("\r\nRechrg Threshold:"));
		SendDig(peep->RechrgThreshold, 2);
	
		Print_P(PSTR("\r\nAbonrml Curr:\t"));
		SendDig(peep->AbnormalChrgCurr, 4);
		Print_P(PSTR("\r\nFast Chrg Curr:\t"));
		SendDig(peep->FastChrgCurr, 4);
		Print_P(PSTR("\r\nSlow Chrg Curr:\t"));
		SendDig(peep->SlowChrgCurr, 4);
		Print_P(PSTR("\r\nNearly Done Curr:"));
		SendDig(peep->NearlyDoneCurr, 4);
		Print_P(PSTR("\r\nChrg Term Curr:\t"));
		SendDig(peep->ChrgTermCurr, 4);

		Print_P(PSTR("\r\nCold Off:\t"));
		SendDig(peep->ColdOff, 3);
		Print_P(PSTR("\r\nCold On:\t"));
		SendDig(peep->ColdOn, 3);
		Print_P(PSTR("\r\nHot On:\t\t"));
		SendDig(peep->HotOn, 3);
		Print_P(PSTR("\r\nHot Off:\t"));
		SendDig(peep->HotOff, 3);
	
		Print_P(PSTR("\r\nCC Threshold:\t"));
		SendDig(peep->CCThreshold, 3);
	
		Print_P(PSTR("\tHealth Threshold: "));
		SendDig(peep->HealthThreshold, 2);
	}
	else
		Print_P(PSTR("\r\nPlease select a slot"));
}

//----------------------------------------------------------------------
//
// Prototype:	PrintBattEEPData2
//
// Description:	Routine to print battery parameters /charger control parameters
//
// Parameters:	None
//
// Returns:		None
//
// Notes:
//
//----------------------------------------------------------------------
void PrintBattEEPData2(void)
{
	if (SYS.DebugSlot < MAX_SLOTS)
	{
		DEBUG_DATA *pdd;
		pdd = DD + SYS.DebugSlot;
		Print_P(PSTR("\r\nP/N:"));
		uint8_t buf[PN_SIZE + 1] = {0};
		memcpy(buf, pdd->PartNum, PN_SIZE);
		Print(buf);
		
		Print_P(PSTR(" S/N:"));
		memcpy(buf, pdd->SerialNum, SN_SIZE);
		buf[SN_SIZE] = 0;
		Print(buf);
		
		Print_P(PSTR("\r\nDate Mfg:"));
		uint16_t year;
		year = (((uint16_t)(pdd->DateMfg[2]&0x0F)<<8) | pdd->DateMfg[1]);
		SendDig(year, 4);
		SendRS232('-');
		SendDig((pdd->DateMfg[2]>>4), 2);
		SendRS232('-');
		SendDig(pdd->DateMfg[0], 2);
		
		EEP_CHRG_DATA *peep;
		peep = EEP + SYS.DebugSlot;
		
		Print_P(PSTR("\r\n       Tmp Curr Volt\r\nRange1:"));
		SendDig(peep->MaxTemp1, 3);
		SendRS232(' ');
		SendDig(peep->FastChrgCurr1, 4);
		SendRS232(' ');
		SendDig(peep->ChrgupVolt1, 6);
		
		Print_P(PSTR("\r\nRange2:"));
		SendDig(peep->MaxTemp2, 3);
		SendRS232(' ');
		SendDig(peep->FastChrgCurr2, 4);
		SendRS232(' ');
		SendDig(peep->ChrgupVolt2, 6);
		
		Print_P(PSTR("\r\nRange3:"));
		SendDig(peep->MaxTemp3, 3);
		SendRS232(' ');
		SendDig(peep->FastChrgCurr3, 4);
		SendRS232(' ');
		SendDig(peep->ChrgupVolt3, 6);
	}
	else
		Print_P(PSTR("\r\nPlease select a slot"));
}