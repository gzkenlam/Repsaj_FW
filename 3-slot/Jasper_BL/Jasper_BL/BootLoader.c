//------------------------------------------------------------------------------------
// FILENAME:  bootloader.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares bootloader functions
//
// %IF Zebra_Internal
//
// NOTES:    
//
// AUTHOR:   		Wasath Mudalige
// CREATION DATE: 	05/03/2016
// DERIVED FROM: 	New File
//
// EDIT HISTORY:
//
//
//
// %End 
//------------------------------------------------------------------------------------
#include <avr/io.h>
#include <avr/lock.h>
#include <avr/signature.h>
#include <string.h>
#include "i2c.h"
#include "uart.h"
#include "macros.h"
#include "timers.h"
#include "flashprog.h"
#include "Jasper_ports.h"

// Set the fuse values
/*
FUSES = 
{
	.byte[0] = FUSE0_DEFAULT,
	.byte[1] = FUSE1_DEFAULT,
	.byte[2] = FUSE_BOOTRST,
	.byte[4] = FUSE4_DEFAULT,
	.byte[5] = FUSE_EESAVE,
};
*/
// Set the lock bits
LOCKBITS = (uint8_t)(BLBB0 & BLBB1); // Bootloader no read/write

FLASH_PROG_PARAM	FP;		// Flash Programing parameters

// Function Prototypes
void DoFlashUpdate(uint8_t bootmode);

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
	uint8_t WasCalled;

	WasCalled = PORTD_DIR;  // Save start reason

	SetupSystemClock();		// Setup the system clocks
	SetupPorts();  			// Setup the I/O ports
	
	// Check if this was not a start from reset
	if (!WasCalled) 		// Was from reset?
	{
		// Yes
		if (DEBUG_PORT & DEBUG_RXD_bm)// If boot loader is not forced by the user
			CheckToasterFirmware();

		DoFlashUpdate(BOOT_MODE1);	// Invalid Toaster firmware or forced entry, enter to boot loader with mode1
	}
	else
		DoFlashUpdate(BOOT_MODE2);	// Entered on I2C command (EnterBL)
}


//------------------------------------------------------------------------------------
//
// Prototype: 	DoFlashUpdate(uint8_t bootmode)
//
// Description:	Routine to take necessary actions as and when I2C registers get updated
//
// Parameters:	bootmode: Boot Loader mode
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void DoFlashUpdate(uint8_t bootmode)
{	
	Init_I2C();		// Setup the I2C interface
	Init_Timers();	// Setup Timers
	Init_Uart();	// Setup Seial UART (115200 8 N 1)
	Init_Variables(bootmode);		// Initialize runtime variables

	// Enable ints
	CPU_CCP = CCP_IOREG_gc;
	PMIC.CTRL = (PMIC_IVSEL_bm | PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm);
	SREG = CPU_I_bm;

	SLEEP_CTRL	= SLEEP_SEN_bm;		// Enable Sleep at idle mode

	while (TRUE)					// Loop forever
	{
		CheckFramesUart();			// Check for boot loader commands through serial UART
		DoEEPFlashUpdate();			// Try to update firmware from external EEPROM

		if (I2C.ActionReq)			// Is TWI action is required?
		{	
			while(Uart.TxBusy);
			uint8_t sreg;
			sreg = SREG;			// Backup STATUS register
			SREG = 0;				// Disable global interrupts
			switch (I2C.ActionReq)
			{	
				case BL_MODE_REG+1:	// Was mode reg updated?					
					if (BL.ModeReg & EXIT_BL_bm)	// Process the command
					{	
						if ((FP.UnwrittenData)  && (READY_TO_WRITE == FP.State))
							WriteUnwrittenPage(FP.BufPtr);	// Write any unwritten data if available
					
						nvm_busy_wait();					// wait till action completes 
						CheckToasterFirmware();				// Validate Toaster firmware, if valid this will not return
						Init_Variables(BOOT_MODE1);			// Reinit working variables
					}
					BL.ModeReg &= (~(ENTER_BL_bm | EXIT_BL_bm));
					break;
				
			}
			I2C.ActionReq = FALSE;			// Action completes
			SREG = sreg;					// Restore STATUS register
		}
		
		SLEEP1;								// Sleep till next interrupt
		NOP;
		NOP;
		NOP;
		NOP;
	}
}

