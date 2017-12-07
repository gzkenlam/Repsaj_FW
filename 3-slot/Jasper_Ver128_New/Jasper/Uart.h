//-----------------------------------------------------------------------------
// FILENAME:  uart.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Uart definitions, data structures and function prototypes are defined in this file
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
#ifndef _UART_H_
#define _UART_H_

//#define _DEBUG_MSGS_

// Defines
#define RX_BUF_SIZE		50		// Uart receive buffer size
#define TX_BUF_SIZE		650		// Uart transmit buffer size

#define CMD_SUCCESS		0x00
#define CMD_INVALID		0x01
#define ACK			0x06
#define NACK		0x15

#define BR_1152		115200UL
#define GET_BAUD_RATE_VAL(F_SYS, F_UART)	2*(((float)F_SYS/ (16*F_UART)) - 1)	// Assume BSCALE = -1

// UART data structure
typedef struct UART_PARAM_TAG
{
	uint8_t RxBuf[RX_BUF_SIZE]; 	// Uart receive ring buffer
	volatile uint8_t Rin, Rout;		// Receive Ring buffer in pointer & out pointer
	volatile uint8_t  RecvData;		// Flag to indicate new data is available in Uart receive buffer

	uint8_t TxBuf[TX_BUF_SIZE];		// Uart transmit ring buffer
	volatile uint16_t Tin,Tout;		// Transmit Ring buffer in pointer & out pointer
	volatile uint8_t  TxBusy;		// Flag to indicate data transmission on progress
} UART_PARAM;

#define MAX_DATA_SIZE	240

typedef struct FRAME_FORMAT_tag
{
	uint8_t BytePos;
	uint8_t ToasterID;
	uint8_t Length;
	uint8_t Cmd;
	uint8_t Data[MAX_DATA_SIZE];
	uint8_t DataCnt;
	uint8_t Cksum;
} FRAME_FORMAT;

typedef enum
{
	FRAME_SYNC = 0,
	TOASTER_ID,
	LENGTH,
	COMMAND,
	DATA,
	CKSUM
} FRAME_POS;

typedef enum
{
	GET_TOASTER_INFO = 0x03,
	SEND_BATT_CHRG_DATA = 0x10,
	SET_BEST_BATT_SLOT,	// 0x11
	SHOW_YOURSELF,		// 0x12
	COPY_FUB_DATA = 0xE0,		// 0xE0
	ENTER_BOOTLOADER = 0xF0,
	WRITE_FLASH_DATA,
	ERASE_FLASH_DATA,
} BL_COMMANDS;

extern UART_PARAM Uart;
extern FRAME_FORMAT	FRAME;

// Function prototypes
void Init_Uart(void);
void Deinit_Uart(void);

uint8_t UartRecvByte(void);
uint8_t AsciiToHex(uint8_t ascii_char);
uint8_t HexToAscii(uint8_t hex_num);

void SendHex(uint8_t hex_num);
void SendLongHex(uint32_t long_hex);
void SendDig(int16_t digit, uint8_t size);
void SendRS232(uint8_t TXByte);
void Print(uint8_t *pstr);
void Print_P(const char *pstr);
void CheckFramesUart(void);
void DoBLCommands(void);
void SendUart(uint8_t *buf, uint8_t len);

#endif // uart.h
