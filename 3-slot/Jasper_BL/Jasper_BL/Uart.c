//------------------------------------------------------------------------------------
// FILENAME:  uart.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares Uart initialization, transmit/receive functions & ISRs
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
#include <string.h>
#include "i2c.h"
#include "uart.h"
#include "macros.h"
#include "flashprog.h"
#include "version.h"

// Global data storage
UART_PARAM Uart;	
FRAME_FORMAT FRAME;
uint8_t ShowMySelf;

//------------------------------------------------------------------------------------
//
// Prototype: 	Init_Uart(void)	
//
// Description:	Routine to initialize Uart interface (115200 bps 8-Bits No Parity 1-Stop bit)
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
void Init_Uart(void)	// Initialize uart
{
	uint8_t trash;
	
	USARTC0_CTRLA = 0;
	USARTC0_CTRLB = 0; 			// Single Speed
	USARTC0_CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
	USARTC0_BAUDCTRLB = 0xF0;	// BScale = -1
    USARTC0_BAUDCTRLA = GET_BAUD_RATE_VAL(F_CPU, BR_1152);	// Baud Select value 115200 bps @ 12MHz

	// Initialize queues and in/out pointers 
 	memset(&Uart, 0, sizeof(Uart));

	USARTC0_CTRLB = USART_RXEN_bm | USART_TXEN_bm;	// Enable receiver & transmitter
	while(USARTC0_STATUS & USART_RXCIF_bm)  		// Flush uart Rx buffer
		trash = USARTC0_DATA;
	
	USARTC0_CTRLA = USART_RXCINTLVL_HI_gc | USART_TXCINTLVL_LO_gc;	// Enable RX/TX interrupts
	memset(&FRAME, 0, sizeof(FRAME_FORMAT));
	ShowMySelf = FALSE;
}

//-----------------------------------------------------------------------------
//
// Prototype: 	Deinit_Uart(void)
//
// Description:	Routine to deinitialize Uart interface
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//-----------------------------------------------------------------------------
void Deinit_Uart(void)
{
	USARTC0_CTRLA = 0;	// Clear Uart regs to there reset values
	USARTC0_CTRLB = 0;
	USARTC0_CTRLC = 6;
	USARTC0_BAUDCTRLB = 0;
	USARTC0_BAUDCTRLA = 0;
}

//-----------------------------------------------------------------------------
//
// Prototype: 	UartRecvByte(void)
//
// Description: Routine to read data from receive queue	
//
// Parameters:	None
//
// Returns:		If unread data is available, returns next data byte in the queue
//				otherwise returns 0
// 
// Notes: 		Call this function if RecvData flag is set
//-----------------------------------------------------------------------------
uint8_t UartRecvByte(void)
{
    uint8_t  sreg, data = 0;
	sreg = SREG;
	SREG = 0;							// Disable interrupts
	if (Uart.Rin != Uart.Rout)			// If unread data is available?
	{
		data = Uart.RxBuf[Uart.Rout++];	// Get the first data byte
		if(Uart.Rout >= RX_BUF_SIZE) 	// If end of buffer then points to beginning of it
   			Uart.Rout = 0;

		if (Uart.Rin == Uart.Rout)		// if last data byte in the queue
			Uart.RecvData = FALSE;		// Clear RecvData flag
	}
	SREG = sreg;						// Restore interrupts
	return data;
}

//-----------------------------------------------------------------------------
//
// Prototype:	SendRS232(uint8_t TXByte)	
//
// Description:	Routine to send one byte through Uart
//
// Parameters:	TXByte - Data byte to be transmit
//
// Returns:		None
// 
// Notes:
//-----------------------------------------------------------------------------
void SendRS232(uint8_t TXByte)
{
	uint8_t sreg;
	uint16_t next_tin;

	sreg = SREG;
	SREG = 0;						// Disable interrupts
	if (Uart.TxBusy)				// If data transmission on progress
	{
		next_tin = Uart.Tin+1;
		if (next_tin == TX_BUF_SIZE)
			next_tin = 0;
		if (next_tin != Uart.Tout)	// if empty space is available in tx buffer?
		{
			Uart.TxBuf[Uart.Tin] = TXByte;	// Place the data in tx buffer
			Uart.Tin = next_tin;	// Update the tx in pointer
		}
	}
	else
	{
		USARTC0_DATA = TXByte;		// Start new data transmission
		Uart.TxBusy = TRUE;			// Set TxBusy flag
	}
	SREG = sreg;					// Restore interrupts
}

//-----------------------------------------------------------------------------
//
// Prototype: 	Print(uint8_t *pstr)
//
// Description:	Routine to print null terminated string from volatile memory(RAM)
//
// Parameters:	pstr - pointer to null terminated string
//
// Returns:		None
// 
// Notes:
//-----------------------------------------------------------------------------
void Print(uint8_t *pstr)
{
	while(1)
	{
		if (!*pstr)
			break;
		else
			SendRS232(*pstr);
		pstr++;
	}
}

//-----------------------------------------------------------------------------
//
// Prototype: 	SIGNAL(USARTC0_RXC_vect)
//
// Description:	Uart data receive ISR
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//-----------------------------------------------------------------------------
SIGNAL(USARTC0_RXC_vect)		// Uart RX ISR
{
    register uint8_t received;
    register uint16_t next_rin;
    
    received = USARTC0_DATA;		// Read the receive byte from UART data Register
    
    next_rin = Uart.Rin + 1;
	if (next_rin >= RX_BUF_SIZE)
		next_rin = 0;

	if (next_rin != Uart.Rout)		// If empty slot is available to store incoming data ?
	{
		Uart.RxBuf[Uart.Rin] = received;	// Store the incoming data in receive buffer
		Uart.Rin = next_rin;		// Update in pointer
		Uart.RecvData = TRUE;		// Set RecvData flag
	}
}

//-----------------------------------------------------------------------------
//
// Prototype: 	SIGNAL(USARTC0_TXC_vect)
//
// Description:	Data Transmit ISR
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//-----------------------------------------------------------------------------
SIGNAL(USARTC0_TXC_vect)	// Uart TX ISR
{
	if (Uart.Tin != Uart.Tout)	// If more data to transmit?
    {
    	USARTC0_DATA = Uart.TxBuf[Uart.Tout++];	// Get the next data byte from tx queue and write to data register.
												// Also increment tx out pointer
    	if (Uart.Tout>=TX_BUF_SIZE)	// If out pointer reached to end of buffer?
    		Uart.Tout = 0;			// Point to beginning of the buffer
    }
    else
    {
    	//ClrBit(USARTC0_CTRLB, USART_TXEN_bp);        // transfer disable
    	//USARTC0_CTRLA  &= (uint8_t)~USART_TXCINTLVL_LO_gc;	// disable Tx interrupt
    	Uart.TxBusy = 0;	// No more data to transmit. Clear Tx busy flag
	}
}

//----------------------------------------------------------------------
//
// Prototype: 	DoBLCommands
//
// Description:	Routine execute LED/Other commands from WT/RS
//
// Parameters:	None
//
// Returns:		None
//
// Notes:
//----------------------------------------------------------------------
void DoBLCommands(void)
{
	uint8_t status = CMD_ERROR;
	uint8_t cksum=0;
	uint8_t *buf;
	buf = &FRAME.BytePos;
	FRAME.BytePos = SYNC_BYTE;
	FRAME.Length = 1;

	switch (FRAME.Cmd)
	{
		case GET_TOASTER_INFO:
			if (!FRAME.DataCnt)
			{
				FRAME.Length = 3 + strlen(JASPER_NAME);
				FRAME.Data[0] = JASPER_3SLOT_ID;
				FRAME.Data[1] = MAX_SLOTS;
				memcpy(&FRAME.Data[2], JASPER_NAME, strlen(JASPER_NAME));
				status = CMD_SUCCESS;
			}
			break;
					
		case SEND_BATT_CHRG_DATA:
			if (!FRAME.DataCnt)
			{
				FRAME.Length = 5;
				FRAME.Data[0] = BL.ModeReg;
				FRAME.Data[1] = JASPER_3SLOT_ID;
				FRAME.Data[2] = BL_MAJOR_VERSION;
				FRAME.Data[3] = BL_MINOR_VERSION;
				status = CMD_SUCCESS;				
			}
			break;
			
		case SHOW_YOURSELF:
			if (FRAME.DataCnt == 1)
			{
				ShowMySelf = FRAME.Data[0];
				status = CMD_SUCCESS;
			}
			break;
					
		case ENTER_BOOTLOADER:
			if (!FRAME.DataCnt)
			{				
				BL.FlashAddress = 0;
				FP.BufPtr = 0;
				FP.Address = 0;
				FP.State = FLASH_PROG_DISABLE;
				status = CMD_SUCCESS;
			}
			break;
			
		case WR_FLASH_DATA:
			if (FRAME.DataCnt == (EEP_PAGE_SIZE + 2))
			{
				// [Byte 0: Address MSB]
				// [Byte 1: Address LSB]
				// [Bytes 2:65: Raw Flash Data]
				uint16_t mem_addr = (((uint16_t)FRAME.Data[0] << 8) | FRAME.Data[1]);
				if (!mem_addr)			// Assume user is trying to initiate a new Firmware update
				{					
					BL.FlashAddress = 0;
					FP.BufPtr = 0;
					FP.Address = 0;
					FP.State = FLASH_PROG_DISABLE;
				}
				
				if (mem_addr == BL.FlashAddress)
				{
					memcpy(&FP.PageBuf[FP.BufPtr], &FRAME.Data[2], EEP_PAGE_SIZE);	// Get a copy of FUB data
					FP.BufPtr += EEP_PAGE_SIZE;
					BL.FlashAddress += EEP_PAGE_SIZE;
					if (SPM_PAGESIZE == FP.BufPtr)
					{// Yes
						if (FLASH_PROG_DISABLE == FP.State)	// If flash was not erased yet
							EraseAllFlashPages();			// Erase application flash

						if (READY_TO_WRITE == FP.State)		// If flash has been erased
						{
							nvm_busy_wait();				// wait till previous action completes
							WritePage();					// Write this page
							ClearPageBuffer();
							FP.BufPtr = 0;					// Reset the buffer pointer
							FP.Address += SPM_PAGESIZE;		// Increment Flash address

							if (BOOT_SECTION_START == FP.Address)	// Are we reached to end of application area?
							{
								FP.State = FLASH_PROG_DONE;	// Yes, we are done with flash programming
								BL.ModeReg = EXIT_BL_bm;
								I2C.ActionReq = BL_MODE_REG + 1;
							}
						}
					}
					status = CMD_SUCCESS;
				}
				
			}
			break;	
	}
	
	FRAME.Cmd = status;
	SendRS232(ACK);
	for (uint8_t cnt = 0; cnt < (FRAME.Length+3); cnt++)
	{
		uint8_t data = *buf;
		SendRS232(data);
		cksum += data;
		buf++;
	}
	SendRS232(-cksum);
	
}
//----------------------------------------------------------------------
//
// Prototype: 	CheckFramesUart
//
// Description:	Routine to decode data received from UART1 and fill into FRAME structure
//
// Parameters:	None
//
// Returns:		None
//
// Notes:
//----------------------------------------------------------------------
void CheckFramesUart(void)
{
	if (Uart.RecvData)							// If new data ready to read
	{
		uint8_t rx_byte = UartRecvByte();		// Read one byte
		FRAME.Cksum += rx_byte;					// Update checksum
		switch (FRAME.BytePos)
		{
			case FRAME_SYNC:
				if (SYNC_BYTE == rx_byte)		// If frame sync byte is correct
				{
					FRAME.Cksum = SYNC_BYTE;
					FRAME.BytePos = TOASTER_ID;	// Move to next step
				}
				break;
			
			case TOASTER_ID:
				if (rx_byte)					// If toaster ID is non-zero
				{
					FRAME.ToasterID = rx_byte;	// Update the Frame with toaster ID
					FRAME.BytePos = LENGTH;		// Move to next step
				}
				else
					FRAME.BytePos = FRAME_SYNC;	// Toaster ID can't be zero. Invalid command. Just discard and go back to beginning
				break;
			
			case LENGTH:
				if (rx_byte)					// If length is non-zero
				{
					FRAME.Length = rx_byte;		// Update the Frame with new length
					FRAME.BytePos = COMMAND;	// Move to next step
				}
				else
					FRAME.BytePos = FRAME_SYNC;	// Length can't be zero. Invalid command. Just discard and go back to beginning
				break;
			
			case COMMAND:
				FRAME.Cmd = rx_byte;			// Store the command
				FRAME.BytePos = DATA;			// Set next step
				FRAME.DataCnt = 0;				// Clear data bytes counter
				break;
			
			case DATA:
				if (FRAME.DataCnt < (FRAME.Length - 1))		// If more data bytes to come
				{
					FRAME.Data[FRAME.DataCnt++] = rx_byte;	// Store them		
					break;
				}
				// No "break" here
			
			case CKSUM:
				if (!FRAME.Cksum)				// If frame checksum is good
					DoBLCommands();				// Do Bootloader command
				else
					SendRS232(NACK);
				FRAME.BytePos = FRAME_SYNC;		// We are done with this frame. Now wait for the next frame
				break;
		}
		
	}
}
