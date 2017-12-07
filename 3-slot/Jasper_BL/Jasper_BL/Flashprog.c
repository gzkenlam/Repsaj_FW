//------------------------------------------------------------------------------------
// FILENAME:  flashprog.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares flash programming utility functions
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
#include <avr/pgmspace.h>
#include "i2c.h"
#include "uart.h"
#include "macros.h"
#include "timers.h"
#include "flashprog.h"
#include "Jasper_ports.h"

FLASH_PROG_PARAM	FP;				// Flash Programing parameters

void exec_nvm_command(uint16_t pgaddr, uint8_t nvmcmd);
//------------------------------------------------------------------------------------
//
// Prototype: 	CheckToasterFirmware(void)
//
// Description:	Routine to validate Toaster firmware, if valid firmware is found jumps it to
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void CheckToasterFirmware(void)
{
	register uint16_t faddr, blank_bytes;
	register uint8_t  fbyte, fcksum;
	
	fcksum = 0;			// Clear the checksum and blank bytes
	blank_bytes = 0;
	
	// This loop reads the flash data from 0x0000 to 0x3FFF and do 8-bit additions to calculate checksum
	for (faddr=0; faddr<BOOT_SECTION_START; faddr++)
	{
		fbyte = pgm_read_byte(faddr);
		fcksum += fbyte;
		if (BLANK_DATA == fbyte)	// If blank byte is found, increment the blank bytes counter
			blank_bytes++;
	}
	if ((!fcksum) && (blank_bytes < BOOT_SECTION_START))	// Check for valid battery firmware
	{
		// Valid Toaster firmware found. jumps to it
		// Disable ints and reset int vectors
		CPU_CCP = CCP_IOREG_gc;
		PMIC.CTRL = 0;
		SREG = 0;
	
		Deinit_Timers();
		Deinit_I2C();
		Deinit_Uart();
		// Restart
		asm volatile ("jmp 0"::);
	}
}

//------------------------------------------------------------------------------------
//
// Prototype: 	ClearPageBuffer(void)
//
// Description:	Routine to clear (write 0xff) page buffer
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void ClearPageBuffer(void)
{
	uint16_t pagptr;
	for (pagptr=0; pagptr<SPM_PAGESIZE; pagptr++)
		FP.PageBuf[pagptr] = BLANK_DATA;
}

//------------------------------------------------------------------------------------
//
// Prototype: 	Init_Variables(uint8_t bootmode)
//
// Description:	Routine to initialize working variables
//
// Parameters:	bootmode - Boot Loader mode:
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void Init_Variables(uint8_t bootmode)
{
	// Init page buffer and working variables
	ClearPageBuffer();
	FP.BufPtr 	= 0;	
	FP.Address 	= 0;
	FP.LastAddress = 0;
	FP.EnKey = EN_KEY2;
	FP.State 	= FLASH_PROG_DISABLE;

	if (I2C.FlashUpdateCnt < MAX_FLASH_UPDATES)
		I2C.ReadExtEEP = START;

	I2C.ActionReq = 0;
	BL.ModeReg = bootmode;
	//if (MCU.DEVID1 == 0x95)
	//	BL.ModeReg |= XMEGA32A_bm;

	BL.FlashAddress = 0;
	//SR.NewData = 0;
}

//------------------------------------------------------------------------------------
//
// Prototype: 	exec_nvm_command(uint16_t pgaddr, uint8_t nvmcmd)
//
// Description:	Routine to execute NVM command
//
// Parameters:	pgaddr - Start address of the flash page
//				nvmcmd - NVM command to be executed (Refer data sheet)
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------

void exec_nvm_command(uint16_t pgaddr, uint8_t nvmcmd)
{
	uint8_t oldcmd;

	while (NVM_STATUS & NVM_FBUSY_bm);  // Wait until NVM controller is ready

	oldcmd = NVM_CMD;	// Backup current NVM command
	NVM_CMD = nvmcmd;  // Set new NVM command
	asm volatile (
		"out	%0, %1\n\t"	// Timed unlock
       	"spm\n\t"			// Trigger erase
		::
		"I" (_SFR_IO_ADDR(CPU_CCP)),
		"d" (CCP_SPM_gc),
		"z" (pgaddr)
		);			

	NVM_CMD = oldcmd;  // Restore previous command
}

//------------------------------------------------------------------------------------
//
// Prototype: 	app_page_erase(uint16_t PageAddr)
//
// Description:	Routine to erase one page of the application area of the flash
//
// Parameters:	PageAddr - Start address of the flash page
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
/*
void app_page_erase(uint16_t PageAddr)
{
	exec_nvm_command(PageAddr, NVM_CMD_ERASE_APP_PAGE_gc);  // Erase page command	
}
*/
//------------------------------------------------------------------------------------
//
// Prototype: 	app_all_erase(void)
//
// Description:	Routine to erase entire application area of the flash
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
/*
void app_all_erase(void)
{
	exec_nvm_command(0, NVM_CMD_ERASE_APP_gc);  // Erase application area	
}
*/
//------------------------------------------------------------------------------------
//
// Prototype: 	nvm_busy_wait(void)
//
// Description:	Routine to check weather NVM is busy with flash programming activity.
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:		This function waits until NVM is free
//------------------------------------------------------------------------------------
void nvm_busy_wait(void)
{
	while (NVM_STATUS & NVM_FBUSY_bm);  // Wait for erase done
}

//------------------------------------------------------------------------------------
//
// Prototype: 	app_erase_page_buffer(void)
//
// Description:	Routine to erase flash page buffer
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void app_erase_page_buffer(void)
{
	uint8_t oldcmd = NVM_CMD;				// Backup current NVM command
	// Erase the flash page buffer
	NVM_CMD = NVM_CMD_ERASE_FLASH_BUFFER_gc;// Load buffer command
	NVM_CTRLA = NVM_CMDEX_bm;
	while (NVM_STATUS & NVM_NVMBUSY_bm);  	// Wait for erase done
	NVM_CMD = oldcmd;  						// Restore previous command
}

//------------------------------------------------------------------------------------
//
// Prototype: 	app_page_fill_buffer(uint16_t PageAddr, uint8_t DataLSB, uint8_t DataMSB)
//
// Description:	Routine to fill page buffer with flash data.A data word is written at a time
//
// Parameters:	PageAddr - Address of the flash page (range 0 to 256 in steps of 2)
//				DataLSB	 - Data (LSB Part)
//				DataMSB	 - Data (MSB Part)
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void app_page_fill_buffer(uint16_t PageAddr, uint8_t DataLSB, uint8_t DataMSB)
{
	uint8_t oldcmd = NVM_CMD;	// Backup current NVM command
	// Load flash page buffer, should be erased by reset or last write
	NVM_CMD = NVM_CMD_LOAD_FLASH_BUFFER_gc;  // Load buffer command
	
	asm volatile (
		"cli\n\t"			// Disable ints since we are messing with r1
		"push	r0\n\t"		// Save r0, r1
		"push	r1\n\t"
		"mov	r0, %0\n\t" // Load data into r1:r0
		"mov	r1, %1\n\t"
		"spm\n\t"			// Trigger write into buffer
		"pop	r1\n\t"		// Restore r0, r1
		"pop	r0\n\t"
		"sei\n\t"			// Enable ints
		::
		"d" (DataLSB),
		"d" (DataMSB),
		"z" (PageAddr)
		: "r0", "r1"
	);
	NVM_CMD = oldcmd;  		// Restore previous command				
}

//------------------------------------------------------------------------------------
//
// Prototype: 	
//
// Description:	
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void app_page_write(uint16_t PageAddr)
{
	// Write flash page
	exec_nvm_command(PageAddr, NVM_CMD_WRITE_APP_PAGE_gc);  // Write page command	
}

//------------------------------------------------------------------------------------
//
// Prototype: 	EraseAllFlashPages(void)
//
// Description:	Routine to erase EA firmware (Entire RWW area from 0x0000 to 0x7FFF)
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:		It takes about 550ms to complete the flash erase. Note that I2C bus 
//				will NOT remains busy during this period
//------------------------------------------------------------------------------------
void EraseAllFlashPages(void)
{
	exec_nvm_command(0, NVM_CMD_ERASE_APP_gc);  // Erase application area	
	FP.UnwrittenData = FALSE;	// Clear unwritten data flag
	FP.State = READY_TO_WRITE;	// Now it is safe to write flash data
	PORTD_OUTCLR = RED_LED_bm;	// Turn off Red LED
	PORTD_OUTSET = GRN_LED_bm;	// Turn on Green LED
	TCD2_CTRLB = PORTD_OUT;		// Enable PWM outputs
}


//------------------------------------------------------------------------------------
//
// Prototype: 	ErasePage(void)
//
// Description:	Routine to erase a page 
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:		It takes about 40ms to complete the page erase.
//------------------------------------------------------------------------------------
/*
void ErasePage(void)
{
	app_page_erase(FP.Address);	// Erase the flash page corresponding to current flash address
	FP.UnwrittenData = FALSE;	// Clear unwritten data flag
	FP.State = READY_TO_WRITE;	// Now it is safe to write flash data
}
*/
//------------------------------------------------------------------------------------
//
// Prototype: 	
//
// Description:	Routine to write flash page. The data to be written should be stored in
//				FP.PageBuf[]. The address of the page should be stored in 'FP.Address'.
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:		This function doesn't wait until page write completes
//------------------------------------------------------------------------------------
void WritePage(void)
{
	uint16_t pagptr;
	
	nvm_busy_wait();			// Just in case, make sure previous write completes
	app_erase_page_buffer();	// Erase page buffer before writing new data
	for(pagptr = 0; pagptr < SPM_PAGESIZE; pagptr += 2) // fill data to Flash buffer
	{
    	app_page_fill_buffer(pagptr, FP.PageBuf[pagptr], FP.PageBuf[pagptr + 1]);
  	}
  	app_page_write(FP.Address);	// write buffer to selected flash page
  	
  	//nvm_busy_wait();			// Don't need this as we are writing to RWW area. The CPU will NOT get halted 
  								// during page writes and therefore it can serve next I2C interrupt. Next page write 
  								// will take place after receiving another 256 bytes which takes at least 50ms
  								// Only exception is when writing any unwritten data (happens when flash address is changed)
  								// So we check the flash busy condition before writing a page. See above
  									
	FP.UnwrittenData = FALSE;	// Clear unwritten data flag
	PORTD_OUTTGL = CHRG_LEDS_gm;// Toggle red and green LEDs at every page write
	TCD2_CTRLB = PORTD_OUT;		// Enable PWM outputs
}

//------------------------------------------------------------------------------------
//
// Prototype: 	WriteUnwrittenPage(uint16_t start)
//
// Description:	Routine to write partially filled flash page. The unfilled portion is written with 0xFF
//
// Parameters:	start - Start address of the page where 0xFF to be written (range: 0 to 256) 
//
// Returns:		None
// 
// Notes:		This function waits until page write completes
//------------------------------------------------------------------------------------
void WriteUnwrittenPage(uint16_t start)
{
	uint16_t pagptr;
	
	for (pagptr=start; pagptr<SPM_PAGESIZE; pagptr++)
		FP.PageBuf[pagptr] = BLANK_DATA;	// Fill blanks on rest of the page
	WritePage();
	nvm_busy_wait();			// wait till this flash page write completes
}


