//-----------------------------------------------------------------------------
// FILENAME:  i2c_batt.c
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
// CREATION DATE: 	03/25/2016
// DERIVED FROM: 	New File

// EDIT HISTORY:
//
//
//
// %End 
//-----------------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include "pwm.h"
#include "uart.h"
#include "macros.h"
#include "timers.h"
#include "i2c_batt.h"
#include "sysparam.h"
#include "battauth.h"
#include "battchrg.h"
#include "battcomm.h"
#include "batt_defs.h"
#include "PP_BattDefs.h"
#include "jasper_ports.h"

volatile TWIM_PARAM I2CBatt;// Global data structure to hold I2C Communication variables
uint8_t *pI2CBattBuf;		// Global pointer to I2C read/ write buffer

//----------------------------------------------------------------------
//
// Prototype:	Init_I2CBatt
//
// Description:	Routine to initialize I2C Master Interface
//
// Parameters:	None
//				
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Init_I2CBatt(void)
{
	TWIC.MASTER.BAUD  = TWI_BAUDRATE(F_CPU, _64KHz);	// Sys Clock = 12MHz, TWI Clock = 64kHz (Must be less than 100kHz)

	// Enable I2C Master and Read/Write interrupts
	TWIC.MASTER.CTRLA = TWI_MASTER_ENABLE_bm | TWI_MASTER_WIEN_bm | TWI_MASTER_RIEN_bm | TWI_MASTER_INTLVL_LO_gc;
	TWIC.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;	// Bring the I2C bus to known state (by force)
	
	I2CBatt.State = I2CMASTER_IDLE;	// Set initial I2C Master state. Other variables don't need to initialize here
}

//----------------------------------------------------------------------
//
// Prototype:	Deinit_I2CBatt
//
// Description:	Routine to deinitialize I2C Master Interface
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Deinit_I2CBatt(void)
{
	TWIC.SLAVE.CTRLA = 0;
	TWIC.SLAVE.ADDR = 0;

	TWIC.MASTER.BAUD  = 0;
	TWIC.MASTER.CTRLA = 0;
	TWIC.MASTER.CTRLB = 0;
	TWIC.MASTER.CTRLC = 0;
	TWIC.MASTER.STATUS = TWI_MASTER_RIF_bm | TWI_MASTER_WIF_bm; // Clear any pending I2C Master interrupts
	
	I2CBatt.State = SHUTDOWN;	// Update the I2C driver status
}

//----------------------------------------------------------------------
//
// Prototype:	Start_I2CBatt
//
// Description:	Routine to start I2C Master transaction
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Start_I2CBatt(void)
{
	pBatt->I2CBusy = TRUE;				// Set battery I2C busy flag
	DISABLE_CLK_BUF;
	SetActives(I2C_BATT_bm);			// Add battery I2C to active peripherals list
	TWIC.MASTER.ADDR = I2CBatt.ChipAddr | TWI_WRITE;	// Start the I2C transaction 
	I2CBatt.State = WRITE_CHIP_ADDR;	// Move to next state
	TCC1_CTRLA = TC_CLKSEL_DIV64_gc;	// Start I2C guard timer (Fclk = 12000/64 = 187.5kHz)
}

//----------------------------------------------------------------------
//
// Prototype:	Update_I2CBattStatus
//
// Description:	Routine to update I2C transaction state (with battery chips)
//
// Parameters:	state - I2C transaction state (ACK_bm / NACK_bm)
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Update_I2CBattStatus(uint8_t state)
{
	TCC1_CTRLA = TC_CLKSEL_OFF_gc;		// Stop I2C guard timer
	UpdateBattCommStatus(state);		// Update battery communication state
	I2CBatt.State = I2CMASTER_IDLE;		// We are done with I2C transaction. Back to idle state
	SYS.SkipSleep = TRUE;				// Don't sleep this time just to ensure that next scheduled I2C transaction starts immediately after this
	SYS.Actives &= ~I2C_BATT_bm;		// Clear I2C battery from active peripherals list
	ENABLE_CLK_BUF;
}

//----------------------------------------------------------------------
//
// Prototype:	Read_I2CAuth
//
// Description:	Routine to read response data from auth chip (508A)
//
// Parameters:	read_cnt: 0x00 to 0xFE: Specify number of bytes to read
//						  0xFF: Figure out the read count from the length byte of the response message
//
// Returns:		None
//
// Notes:
//----------------------------------------------------------------------
void Read_I2CAuth(uint8_t read_cnt)
{
	I2CBatt.ChipAddr |=  TWI_READ;		// Set the I2C slave address
	I2CBatt.Pointer  = 0;				// Not applicable
	I2CBatt.ReadCnt  = read_cnt;		// Set the read byte count
	I2CBatt.WriteCnt = 0;				// Not applicable
	pI2CBattBuf = CommBuf;				// Set the pointer to read/write buffer
	
	pBatt->I2CBusy = TRUE;
	DISABLE_CLK_BUF;
	SetActives(I2C_BATT_bm);			// Add battery I2C to active peripherals list
	TWIC.MASTER.ADDR = I2CBatt.ChipAddr;// Start the I2C transaction
	I2CBatt.State = READ_DATA;			// Move to next state
	
	TCC1_CNT = 0;
	TCC1_CTRLA = TC_CLKSEL_DIV64_gc;	// Start I2C guard timer (Fclk = 12000/64 = 187.5kHz)
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

	I2CBattStatus = TWIC.MASTER.STATUS;

	// Check for bus error or arbitration lost condition
	if ((I2CBattStatus & TWI_MASTER_ARBLOST_bm) || (I2CBattStatus & TWI_MASTER_BUSERR_bm))
	{
		// Shouldn't happen in single master mode
		TWIC.MASTER.STATUS |= (TWI_MASTER_ARBLOST_bm | TWI_MASTER_BUSERR_bm);	// Clear error flags
		TWIC.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;	// Bring the I2C bus to known state (by force)
		Update_I2CBattStatus(NACK_bm);
	}
	else	// No bus errors
	{

		if (I2CBattStatus & TWI_MASTER_WIF_bm)				// If master write
		{
			if (I2CBattStatus & TWI_MASTER_RXACK_bm)		// If slave NACK
			{
				TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;	// Send STOP
				if ((WRITE_POINTER == I2CBatt.State) && (pBatt->MiscFlags & BATT_VERIFIED_bm))			// If slave NACKed to pointer after the battery verification
				{// Yes and its gas gauge address
					if ((GG_SLV_ADDR_QLN == I2CBatt.ChipAddr) || (GG_SLV_ADDR_ZQ3 == I2CBatt.ChipAddr))
						Update_I2CBattStatus(ACK_bm);		// Just skip this time
					else
						Update_I2CBattStatus(NACK_bm);		// Send NACK status
				}
				else
					Update_I2CBattStatus(NACK_bm);		// Send NACK status
			} 
			else	// If slave ACK
			{
				if (WRITE_CHIP_ADDR == I2CBatt.State)
				{
					TWIC.MASTER.DATA = I2CBatt.Pointer;		// Send the pointer
					I2CBatt.State = WRITE_POINTER;
				}
				else 
					if (WRITE_POINTER == I2CBatt.State)
					{
						if (I2CBatt.WriteCnt)				// If data write
						{
							TWIC.MASTER.DATA = *pI2CBattBuf;// Write data
							pI2CBattBuf++;
							I2CBatt.WriteCnt--;				// Decrement byte counter
						}
						else
						{ 
							if (I2CBatt.ReadCnt)			// If data need to be read
							{
								TWIC.MASTER.ADDR = I2CBatt.ChipAddr | TWI_READ;	// Initiate master read
								I2CBatt.State = READ_DATA;
							}
							else	// If last data byte was written
							{
								TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;	// Send STOP condition
								Update_I2CBattStatus(ACK_bm);
							}
						}
					
					}
			}
			TWIC.MASTER.STATUS |= TWI_MASTER_WIF_bm;
		} 
		else	// If master read
		{
			if (I2CBattStatus & TWI_MASTER_RIF_bm)
			{
				if (BLOCK_READ == I2CBatt.ReadCnt)
				{
					I2CBatt.ReadCnt = TWIC.MASTER.DATA;			// Store the data in read buffer
					if (I2CBatt.ReadCnt > 30)
						I2CBatt.ReadCnt = 30;
					if (!I2CBatt.ReadCnt)
					{
						TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc | TWI_MASTER_ACKACT_bm;	// Send NACK and then STOP condition
						Update_I2CBattStatus(ACK_bm);
					}
					else
						TWIC.MASTER.CTRLC = TWI_MASTER_CMD1_bm;	// Send ACK
				}
				else
				{
					*pI2CBattBuf = TWIC.MASTER.DATA;			// Store the data in read buffer
					if (I2CBatt.ReadCnt == 0xFF)
						I2CBatt.ReadCnt = *pI2CBattBuf;
						
					pI2CBattBuf++;
					if (--I2CBatt.ReadCnt)						// For the all read bytes expect the last one
						TWIC.MASTER.CTRLC = TWI_MASTER_CMD1_bm;	// Send ACK
					else										// We have read the last byte
					{
						// If this is a FUB read
						if (FUB_SLV_ADDR == I2CBatt.ChipAddr)	// If this is reading data from FUB
							EncryptDecryptBattData(DECRYPT_DATA);	// Decrypt the data
						else
						{
							if ((AUTH_SLV_ADDR_MLB | TWI_READ) == (I2CBatt.ChipAddr & (AUTH_SLV_ADDR_MLB | TWI_READ)))	// If this is reading data from an Auth chip
								CheckAuthResponse();				// Check the CRC and copy data to response buffer
						}
						TWIC.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc | TWI_MASTER_ACKACT_bm;	// Send NACK and then STOP condition
						Update_I2CBattStatus(ACK_bm);
					}
				}
			}
		}
	}
}

//----------------------------------------------------------------------
//
// Prototype: 	SIGNAL (TCC1_OVF_vect)
//
// Description:	I2C Master Timeout
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:		This interrupt occurs if the an I2C master transaction doesn't complete within guard time (280ms)
//----------------------------------------------------------------------
SIGNAL(TCC1_OVF_vect)
{
	// I2C transaction timeout. Something wrong with the I2C h/w interface!
#ifdef _DEBUG_MSGS_
	SendRS232('T');
	SendHex(I2CBatt.ChipAddr);
	SendHex(I2CBatt.Pointer);
#endif
	Deinit_I2CBatt();	// Reinitialize I2C battery interface
	Init_I2CBatt();
	Update_I2CBattStatus(NACK_bm);	// Update the status with NACK	
}
