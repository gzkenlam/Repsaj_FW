//------------------------------------------------------------------------------------
// FILENAME:  main.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares main program
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
//------------------------------------------------------------------------------------

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "a2d.h"
#include "led.h"
#include "pwm.h"
#include "uart.h"
#include "debug.h"
#include "macros.h"
#include "eeprom.h"
#include "timers.h"
#include "version.h"
#include "i2c_batt.h"
#include "sysparam.h"
#include "battchrg.h"
#include "battcomm.h"
#include "batt_defs.h"
#include "battfound.h"
#include "PP_BattDefs.h"
#include "jasper_ports.h"

// For field firmware update verification purpose store device ID and firmware versions at the very last addresses of the flash memory
// 0x3FFC: Product ID, 0x3FFD: Major Version, 0x3FFE: Minor Version
uint8_t ToasterFWVer[3] __attribute__ ((section (".FWVersion"))) = {JASPER_3SLOT_ID, VERSION_MAJOR, VERSION_MINOR};

SYSTEM_PARAM SYS;	// Global data structure to hold system variables

//------------------------------------------------------------------------------------
//
// Prototype: 	main()
//
// Description:	main program executed after a reset
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
int main()
{
	SetupSystemClock();	// Setup the system clocks
	SetupPorts();  		// Setup the I/O ports

	// Shutdown unused peripherals
	PR_PRGEN = PR_AES_bm | PR_EBI_bm |PR_EVSYS_bm | PR_DMA_bm;
	PR_PRPA  = PR_DAC_bm;
	PR_PRPB  = PR_DAC_bm | PR_ADC_bm | PR_AC_bm;
	PR_PRPC  = PR_USART1_bm | PR_SPI_bm | PR_HIRES_bm;
	PR_PRPD  = PR_USART0_bm | PR_USART1_bm | PR_HIRES_bm;
	PR_PRPE  = PR_USART0_bm | PR_HIRES_bm;

	Init_FreeRAM();
	Init_ExtInts();
	Init_Uart();		// Setup the Serial Uart interface
	Init_A2D();			// Setup the A2D Converter
	Init_LEDs();
	Init_I2CBatt();		// Setup the I2C Slave interface
	Init_BattCharger();
	Init_Debug();
	Init_PWM();
	Init_Timers();		// Setup RTC Timer

	CLEAR_PENDING_OVER_CURR_INTS;
	ENABLE_OVER_CURR_INT;

	// Enable ints
	PMIC.CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
	SREG = CPU_I_bm;

	SLEEP_CTRL	= SLEEP_SEN_bm;	// Enable Sleep at idle mode

	PrintVersionString();// Send initial debug messages

	while (TRUE)	// Loop forever
	{	
		if (SYS.ModeReg & ENTER_BL_bm)	// If enter boot loader command received
		{
			while (Uart.TxBusy);		// Wait till remaining debug messages are sent out
			SREG = 0;					// Disable global interrupts
			// De initialize active peripherals
			Deinit_Timers();
			Deinit_LEDs();
			Deinit_A2D();
			Deinit_Uart();
			Deinit_I2CBatt();
			Deinit_PWM();
			Deinit_ExtInts();
			
			CPU_CCP = CCP_IOREG_gc;
			PMIC.CTRL = 0;

			// Restart
			//if (MCU.DEVID1 == 0x95)				// If this is a 32K Part
			//	asm volatile ("jmp 0x8000"::); 	// Jump to boot loader start (32K Part)
			
			asm volatile ("jmp 0x8000"::); 		// Jump to boot loader start (16K part)		
		}
		
		ScanChrgSlots();			// Scan each charger slot for battery charging
		DoLEDPatterns();			// Do LED Patterns
		CheckFramesUart();			// Check for bootloader/FUB commands through serial UART
		
		if (SYS.NextSlot)			// If it's time to serve the next slot
		{
			SetupBattChrgData();	// Setup debug messages
			if (!SYS.DebugCnt)		// If it's time to print debug messages
				PrintBattChrgInfo();// Print debug messages
				
			// We are about to switch to next slot, however I2C busy flag is still set which is not good
			while (pBatt->I2CBusy);	// Wait till I2C busy flag to clear

			SYS.NextSlot = FALSE;	// Clear next slot flag
			
			if (++PORTR_OUT == MAX_SLOTS)
				PORTR_OUT = SLOT0;
			
			if (!PORTR_OUT)			// If all slots are served
			{
			#ifdef _BEST_BATT_
				FindBestBattSlot();	// Find the best battery slot
			#endif
				pBatt = Batt;		// Initialize pointers to battery and eeprom data structures
				pEEP = EEP;
				SYS.ScanSlots = FALSE;	// Clear scan slots flag
			}
			else
			{
				pBatt++;	// Switch to next slot
				pEEP++;
				continue;	// Don't sleep, scan the remaining slots
			}
		}
		if (SYS.SkipSleep)	// If there is a request not to sleep
		{
			SYS.SkipSleep = FALSE;
			continue;		// Don't sleep
		}
		else
		{
			uint8_t sreg = SREG;
			SREG = 0;		// Disable interrupts
			if ((!SYS.Actives) && (!SYS.PWMActives))	// If non of the peripherals are active
				SLEEP_CTRL = SLEEP_SMODE_PSAVE_gc | SLEEP_SEN_bm;	// Good to go to Power Save Mode

			SREG = sreg;	// Enable interrupts
		}
		SLEEP1;				// Sleep till next interrupt
		NOP;				// Usually add few NOP instructions after sleep
  		NOP;
  		NOP;
  		NOP;
	} 
}
