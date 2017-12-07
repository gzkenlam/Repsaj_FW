//-----------------------------------------------------------------------------
// FILENAME:  i2c_batt.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: I2C Master definitions, data structures and function prototypes are defined in this file
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
#ifndef I2C_BATT_H
#define I2C_BATT_H

#define _31P25KHz			31250UL		// I2C Master clock speed = 31.25kHz (Same as MPA2)
#define _64KHz				64000UL		// I2C Master clock speed = 64 kHz
#define _100KHz				100000UL

#define ACK_bm				0x80
#define NACK_bm				0x40

#define TWI_READ			1
#define TWI_WRITE			0
#define TWI_BAUDRATE(F_SYS, F_TWI)	((F_SYS/(2*F_TWI)) - 5)	// Baudrate formula mentioned in data sheet

#define I2CMASTER_IDLE		0
#define WRITE_CHIP_ADDR		1
#define WRITE_POINTER		2
#define READ_DATA			3
#define WRITE_DATA			4
#define SHUTDOWN			5

#define BLOCK_READ			0xFE

// I2C Master variable structure
typedef struct TWIM_PARAM_tag
{
	uint8_t State;
	uint8_t ChipAddr;
	uint8_t Pointer;
	uint16_t ReadCnt;
	uint16_t WriteCnt;
} TWIM_PARAM;

extern volatile TWIM_PARAM I2CBatt;
extern uint8_t *pI2CBattBuf;

// Function Prototypes
void Init_I2CBatt(void);
void Deinit_I2CBatt(void);
void Start_I2CBatt(void);
void Update_I2CBattStatus(uint8_t state);
void Read_I2CAuth(uint8_t read_cnt);

#endif	//I2C_Batt.h
