//-----------------------------------------------------------------------------
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
#include "uart.h"
#include "macros.h"
#include "debug.h"
#include "sysparam.h"

UART_PARAM Uart;	// Global structure to hold UART parameters
FRAME_FORMAT	FRAME;

//----------------------------------------------------------------------
//
// Prototype: 	Init_Uart
//
// Description:	Routine to setup Uart interface (38400bps 8bits No Parity, 1-Stop bit)
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Init_Uart(void)	// Initialize uart
{
	//uint8_t trash;
	
	USARTC0_CTRLA = 0;
	USARTC0_CTRLB = 0; 			// Single Speed
	USARTC0_CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
	USARTC0_BAUDCTRLB = 0xF0;	// BScale = -1
	
    USARTC0_BAUDCTRLA = GET_BAUD_RATE_VAL(F_CPU, BR_1152);
	// Initialize queues and in/out pointers 
 	memset(&Uart, 0, sizeof(Uart));
	memset(&FRAME, 0, sizeof(FRAME_FORMAT));
   
	USARTC0_CTRLB = USART_RXEN_bm | USART_TXEN_bm;	// Enable receiver & transmitter
	//while(USARTC0_STATUS & USART_RXCIF_bm)  		// Flush uart Rx buffer
	//	trash = USARTC0_DATA;
	
	USARTC0_CTRLA = USART_RXCINTLVL_HI_gc | USART_TXCINTLVL_LO_gc;	// Enable RX/TX interrupts
}


//----------------------------------------------------------------------
//
// Prototype: 	Deinit_Uart
//
// Description:	Routine to deinitialize Uart interface
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Deinit_Uart(void)
{
	USARTC0_CTRLA = 0;	// Clear Uart regs to there reset values
	USARTC0_CTRLB = 0;
	USARTC0_CTRLC = 6;
	USARTC0_BAUDCTRLB = 0;
	USARTC0_BAUDCTRLA = 0;
	USARTC0_STATUS = USART_RXCIF_bm | USART_TXCIF_bm | USART_DREIF_bm;
}

//----------------------------------------------------------------------
//
// Prototype: 	UartRecvByte
//
// Description: Routine to read data from receive queue	
//
// Parameters:	None
//
// Returns:		If unread data is available, returns next data byte in the queue
//				otherwise returns 0
// 
// Notes: 		Call this function if RecvData flag is set
//----------------------------------------------------------------------
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

//----------------------------------------------------------------------
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
//----------------------------------------------------------------------
SIGNAL(USARTC0_RXC_vect)		// Uart RX ISR
{
    register uint8_t received;
    register uint16_t next_rin;
    
    received = USARTC0_DATA;		// Read the receive byte from UART data Register
    
    next_rin = Uart.Rin + 1;
	if (next_rin >= RX_BUF_SIZE)
		next_rin = 0;

	if (next_rin != Uart.Rout)		// If empty slot is available to store incoming data?
	{
		Uart.RxBuf[Uart.Rin] = received;	// Store the incoming data in receive buffer
		Uart.Rin = next_rin;		// Update in pointer
		Uart.RecvData = TRUE;		// Set RecvData flag
	}
}

//----------------------------------------------------------------------
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
//----------------------------------------------------------------------
SIGNAL(USARTC0_TXC_vect)	// Uart TX ISR
{
	if (Uart.Tin != Uart.Tout)	// If more data to transmit?
    {
    	USARTC0_DATA = Uart.TxBuf[Uart.Tout++];	// Get the next data byte from tx queue and write to data register.
												// Also increment tx out pointer
    	if (Uart.Tout>=TX_BUF_SIZE)				// If out pointer reached to end of buffer?
    		Uart.Tout = 0;						// Point to beginning of the buffer
    }
    else
    {
		//USARTC0_CTRLA = USART_RXCINTLVL_LO_gc;	// disable Tx interrupt
    	//USARTC0_CTRLB = USART_RXEN_bm;        	// transfer disable
    	Uart.TxBusy = 0;						// No more data to transmit. Clear Tx busy flag
		SYS.Actives &= ~UART_TX_bm;				// Clear uart TX flag from active peripheral list
	}
}

//----------------------------------------------------------------------
//
// Prototype: 	Print
//
// Description:	Routine to print null terminated string from volatile memory(RAM)
//
// Parameters:	pstr - pointer to null terminated string
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Print(uint8_t *pstr)
{
	while(TRUE)
	{
		if (!*pstr)
			break;
		else
			SendRS232(*pstr);
		pstr++;
	}
}

//----------------------------------------------------------------------
//
// Prototype:	Print_P
//
// Description:	Routine to print null terminated string from non-volatile memory (Flash)
//
// Parameters:	pstr - pointer to null terminated string
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Print_P(const char *pstr)
{
	uint8_t pbyte;
	while (TRUE)
	{
		pbyte = pgm_read_byte(pstr);
		if (!pbyte)
			break;
		else
			SendRS232(pbyte);
		pstr++;
	}
}

//----------------------------------------------------------------------
//
// Prototype:	SendRS232
//
// Description:	Routine to send one byte through Uart
//
// Parameters:	TXByte - Data byte to be transmit
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void SendRS232(uint8_t TXByte)
{
	uint8_t sreg;
	uint16_t next_tin;

	sreg = SREG;
	SREG = 0;						// Disable interrupts
	if (Uart.TxBusy)				// If data transmission on progress
	{
		next_tin = Uart.Tin+1;		
		if (TX_BUF_SIZE == next_tin)
			next_tin = 0;
		if (next_tin != Uart.Tout)	// if empty space is available in tx buffer?
		{
			Uart.TxBuf[Uart.Tin] = TXByte;	// Place the data in tx buffer
			Uart.Tin = next_tin;	// Update the tx in pointer
		}
	}
	else							
	{	
		//USARTC0_CTRLB  = USART_RXEN_bm | USART_TXEN_bm;	// Enable receiver & transmitter
		//USARTD1.STATUS = USART_TXCIF_bm | USART_DREIF_bm;
		//USARTC0_CTRLA  = USART_RXCINTLVL_LO_gc | USART_TXCINTLVL_LO_gc;	// Enable RX/TX interrupts

		SLEEP_CTRL = SLEEP_SEN_bm;	// Enable Sleep at idle mode
		SYS.Actives |= UART_TX_bm;	// Add uart transmitter to active peripheral list

		USARTC0_DATA = TXByte;		// Start new data transmission
		Uart.TxBusy = TRUE;			// Set TxBusy flag
	}
	SREG = sreg;					// Restore interrupts
}

//----------------------------------------------------------------------
//
// Prototype:	SendHex
//
// Description:	Routine to print hex number 
//
// Parameters:	hex_num - hex value to be print (0x00 to 0xFF)
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void SendHex(uint8_t hex_num)
{
	uint8_t hstr[4];
	hstr[0] = HexToAscii(hex_num>>4);
	hstr[1] = HexToAscii(hex_num&0xf);
	hstr[2] = ' ';
	hstr[3] = 0;
	Print(hstr);
}

//----------------------------------------------------------------------
//
// Prototype:	SendLongHex
//
// Description:	Routine to print long hex number 
//
// Parameters:	long_hex - hex value to be print (0x00000000 to 0xFFFFFFFF)
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------

void SendLongHex(uint32_t long_hex)
{
	uint8_t hstr[9];
	hstr[0] = HexToAscii(long_hex>>28);
	hstr[1] = HexToAscii((long_hex>>24)&0xf);
	hstr[2] = HexToAscii((long_hex>>20)&0xf);
	hstr[3] = HexToAscii((long_hex>>16)&0xf);
	hstr[4] = HexToAscii((long_hex>>12)&0xf);
	hstr[5] = HexToAscii((long_hex>>8)&0xf);
	hstr[6] = HexToAscii((long_hex>>4)&0xf);
	hstr[7] = HexToAscii(long_hex&0xf);
	hstr[8] = 0;
	Print(hstr);
}

//----------------------------------------------------------------------
//
// Prototype:	SendDig
//
// Description:	Routine print an integer in decimal format
//
// Parameters:	digit - integer to be print (-32768 to 32767)
//				size  - number of digits to be displayed (1 to 6)
//						1:     -9 to 9
//						2:    -99 to 99
//						3:   -999 to 999
//						4:  -9999 to 9999
//						5: -9.999 to 9.999
//						6:-32.768 to 32.767
//
// Returns:	None
// 
// Notes:
//----------------------------------------------------------------------
void SendDig(int16_t digit, uint8_t size)
{
	uint8_t digitbuf[7];
	uint8_t pos;
	uint8_t decpos = 7;

	if ((!size) || (size > 6))	// Check for valid size range
		return;
	
	if (digit<0)		// If negative value
	{
		digit *= -1;	// Make it positive
		SendRS232('-');	// Print the negative sign
	}

	if (size>4)			// Put the decimal place if size > 4
	{
		decpos = size-3;
		digitbuf[decpos-1] = '.';
	}	
	for (pos=size; pos>0; pos--)
	{
		if (pos != decpos)
		{
			digitbuf[pos-1] = digit%10+'0';
			digit = digit/10;
		}
	}
	digitbuf[size]=0;	// Null terminate string
	Print(digitbuf);
}

//----------------------------------------------------------------------
//
// Prototype:	HexToAscii
//
// Description:	Routine to convert hex number in to Ascii character
//
// Parameters:	hex_num - hex number to be converted (expected 0x0 to 0xF but not limited to)
//
// Returns:		Corresponding Ascii character (e.g 0 -> '0')
// 
// Notes:		No range checking for input parameter
//----------------------------------------------------------------------
uint8_t HexToAscii(uint8_t hex_num)
{
	if (hex_num < 10)
		return(hex_num + '0');
	return(hex_num + 'A'-10);
}

//----------------------------------------------------------------------
//
// Prototype:	AsciiToHex
//
// Description:	Routine to convert Ascii character to hex number
//
// Parameters:	ascii_char - Ascii character (expected '0'to '9' or 'A' to 'F' but not limited to)
//
// Returns:		Corresponding hex value (e.g '0'->0)
//
// Notes:		No range checking for input parameter
//----------------------------------------------------------------------
uint8_t AsciiToHex(uint8_t ascii_char)
{
	if ((ascii_char >= '0') && (ascii_char <= '9'))
	return (ascii_char-'0');
	else
	ascii_char |= 0x20; // Make it lower case
	
	return (ascii_char - 'a'+10);
}

//----------------------------------------------------------------------
//
// Prototype: 	CheckFramesUart
//
// Description:	Routine to decode data received from UART and fill into FRAME structure
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
				else
					DoDebugCmds(rx_byte);		// Handle debug commands
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