//------------------------------------------------------------------------------------
// FILENAME:  i2c.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares functions and ISR of I2C Master mode operation
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
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "i2c.h"
#include "macros.h"
#include "version.h"
#include "flashprog.h"
#include "Jasper_ports.h"

// Global storage - I2C registers
I2C_PARAM	BL;
TWIS_PARAM	I2C={0};

//------------------------------------------------------------------------------------
//
// Prototype: 	Init_I2C(void)
//
// Description:	Routine to setup I2C Master interface (PORTE)
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void Init_I2C(void)
{
	// Init state machine and data
	memset(&BL, 0, sizeof(I2C_PARAM));

	BL.MajorVer = BL_MAJOR_VERSION;
	BL.MinorVer = BL_MINOR_VERSION;

	TWIC_MASTER_BAUD  = TWI_BAUDRATE(F_CPU, _64KHz);	// Sys Clock = 12MHz, TWI Clock = 64kHz (Must be less than 100kHz)

	// Enable I2C Master and Read/Write interrupts
	TWIC_MASTER_CTRLA = TWI_MASTER_ENABLE_bm | TWI_MASTER_WIEN_bm | TWI_MASTER_RIEN_bm | TWI_MASTER_INTLVL_LO_gc;
	TWIC_MASTER_STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;	// Bring the I2C bus to known state (by force)
	
	I2C.State = I2CMASTER_IDLE;	// Set initial I2C Master state. Other variables don't need to initialize here
}

//------------------------------------------------------------------------------------
//
// Prototype: 	Init_I2C(void)
//
// Description:	Routine to deinitialize I2C Master interface (PORTE)
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void Deinit_I2C(void)
{
	TWIC_MASTER_BAUD  = 0;
	TWIC_MASTER_CTRLA = 0;
}

//----------------------------------------------------------------------
//
// Prototype:	Update_I2CStatus
//
// Description:	Routine to update I2C transaction state with the EEPROM
//
// Parameters:	state - I2C transaction state (ACK_bm / NACK_bm)
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Update_I2CStatus(uint8_t state)
{
	ENABLE_CLK_BUF;					// Enable I2C Clock buffer
	I2C.State = I2CMASTER_IDLE;		// We are done with I2C transaction. Back to idle state
	I2C.ReadExtEEP = START;			// Move to next I2C state
	if (ACK_bm == state)			// The I2C transaction was successful
	{// Yes	
		if (SPM_PAGESIZE == FP.BufPtr)
		{// Yes
			if (FLASH_PROG_DISABLE == FP.State)	// If flash was not erased yet
				EraseAllFlashPages();			// Erase application flash

			if (READY_TO_WRITE == FP.State)		// If flash has been erased
			{
				DecryptToasterFirmware();		// Decrypt the Toaster firmware before writing to the flash
				nvm_busy_wait();				// wait till previous action completes 
				WritePage();					// Write this page
				ClearPageBuffer();
				FP.BufPtr = 0;					// Reset the buffer pointer
				FP.Address += SPM_PAGESIZE;		// Increment Flash address
				FP.EnKey ^= EN_KEY2;

				if (BOOT_SECTION_START == FP.Address)	// Are we reached to end of application area?
				{
				 	FP.State = FLASH_PROG_DONE;	// Yes, we are done with flash programming
					I2C.ReadExtEEP = DONE;		// Move to next I2C state
					I2C.FlashUpdateCnt++;
					BL.ModeReg = EXIT_BL_bm;
					I2C.ActionReq = BL_MODE_REG + 1;
				}
			}
		}
		I2C.RetryCnt = 0;	// Reset retry count
	}
	else
	{
		if (++I2C.RetryCnt == MAX_RETRY_CNT)	// If all retries are over
		{
			I2C.RetryCnt = 0;					// Reset retry count
			I2C.ReadExtEEP = FAILED;			// Set read eeprom state as failed
			if (FP.State != FLASH_PROG_DISABLE)	// If failure occur while flash update is on progress
				Init_Variables(BOOT_MODE1);		// Initialize run time variables
		}
	}
}

//----------------------------------------------------------------------
//
// Prototype: 	SIGNAL (TWIC_TWIM_vect)
//
// Description:	I2C Master ISR
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
SIGNAL (TWIC_TWIM_vect)
{
	register uint8_t I2CBattStatus;

	I2CBattStatus = TWIC_MASTER_STATUS;

	// Check for bus error or arbitration lost condition
	if ((I2CBattStatus & TWI_MASTER_ARBLOST_bm) || (I2CBattStatus & TWI_MASTER_BUSERR_bm))
	{
		// Shouldn't happen in single master mode
		TWIC_MASTER_STATUS |= (TWI_MASTER_ARBLOST_bm | TWI_MASTER_BUSERR_bm);	// Clear error flags
		TWIC_MASTER_STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;	// Bring the I2C bus to known state (by force)
		Update_I2CStatus(NACK_bm);
	}
	else	// No bus errors
	{

		if (I2CBattStatus & TWI_MASTER_WIF_bm)				// If master write
		{
			if (I2CBattStatus & TWI_MASTER_RXACK_bm)		// If slave NACK
			{
				TWIC_MASTER_CTRLC = TWI_MASTER_CMD_STOP_gc;	// Send STOP
				Update_I2CStatus(NACK_bm);
			} 
			else	// If slave ACK
			{
				if (WRITE_CHIP_ADDR == I2C.State)
				{
					TWIC_MASTER_DATA = (BL.FlashAddress>>8);		// Send 1st address
					I2C.State = WRITE_FIRST_ADDR;
				}
				else 
					if (WRITE_FIRST_ADDR == I2C.State)
					{
						TWIC_MASTER_DATA = BL.FlashAddress&0xFF;		// Send 2nd address
						I2C.State = WRITE_SECOND_ADDR;
					}
					else
						{ 
							TWIC_MASTER_ADDR = EEP_SLAVE_ADDR | TWI_READ;	// Initiate master read
							I2C.State = READ_EEP_DATA;	
						}
			}
			TWIC_MASTER_STATUS |= TWI_MASTER_WIF_bm;
		} 
		else	// If master read
		{
			if (I2CBattStatus & TWI_MASTER_RIF_bm)
			{
				FP.PageBuf[FP.BufPtr++] = TWIC_MASTER_DATA;			// Store the data in read buffer
				BL.FlashAddress++;					// Increment the flash address
				if ((FP.BufPtr % EEP_PAGE_SIZE) == 0)
				{
					TWIC_MASTER_CTRLC = TWI_MASTER_CMD_STOP_gc | TWI_MASTER_ACKACT_bm;	// Send NACK and then STOP condition
					Update_I2CStatus(ACK_bm);
				}
				else
					TWIC_MASTER_CTRLC = TWI_MASTER_CMD1_bm;	// Send ACK
			}
		}
	}
}

//----------------------------------------------------------------------
//
// Prototype:	DecryptToasterFirmware
//
// Description:	Routine to decrypt 256 bytes of Toaster firmware
//
// Parameters:	R16: Action to be performed (0:Decrypt, 1:Encrypt, 2:Decrypt without clearing last bye)
//				X Reg: RAM address of data buffer
//				Y Reg: Address of the EEPROM (YL:0x00~0xFF, YH=0x00 or 0x02)
//				
// Returns:		None
// 
// Notes: This is a blocking call. 
//----------------------------------------------------------------------
void DecryptToasterFirmware(void)
{
	register uint8_t action asm ("r16");// Use exclusively R16 for this variable
	action = DECRYPT_WITH_LAST_BYTE;	// Decrypt data but don't clear very last byte
	uint16_t addr = FP.EnKey;
	uint16_t faddr = AESEncryptDecrypt32K;// Set the flash address of the encryption routine in bootloader area
	uint8_t *pBuf;
	
	pBuf = FP.PageBuf;			// Set buffer position
	for (uint8_t cnt=0; cnt<BLOCKS_PER_SPMPAGE; cnt++)
	{
		asm volatile (	"icall\n\t"			// Call the decryption routine
						:: 
						"r" (action),
						"z" (faddr),
						"y" (addr),
						"x" (pBuf)			// Store the (RAM) address in X register
						);
		*pBuf ^= ((MAGIC_NUMBER ^ JASPER_3SLOT_ID)+cnt);		// Rollback byte shuffling
		*(pBuf+1) ^= (FP.Address >> 8);
		pBuf += EEP_BLOCK_SIZE;
	}
	
}

//----------------------------------------------------------------------
//
// Prototype:	DoEEPFlashUpdate
//
// Description:	Routine to update Toaster firmware from external EEPROM (24C128)
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void DoEEPFlashUpdate(void)
{
	if ((START == I2C.ReadExtEEP) && (I2CMASTER_IDLE == I2C.State))
	{
		I2C.ReadExtEEP = STOP;
		DISABLE_CLK_BUF;				// Disable I2C Clock buffer
		TWIC_MASTER_ADDR = EEP_SLAVE_ADDR | TWI_WRITE;	// Start the I2C transaction 
		I2C.State = WRITE_CHIP_ADDR;	// Move to next state	
	}
}
