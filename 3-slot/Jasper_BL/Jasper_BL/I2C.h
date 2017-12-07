//-----------------------------------------------------------------------------
// FILENAME:  i2c.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: I2C Register definitions, data structures and function prototypes are defined in this file
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
//-----------------------------------------------------------------------------
#ifndef I2C_H
#define I2C_H

#define _CLK_BUF_

#define DECRYPT_WITH_LAST_BYTE		2
#define EEP_BLOCK_SIZE				16
#define BLOCKS_PER_SPMPAGE			(SPM_PAGESIZE/EEP_BLOCK_SIZE)
#define AESEncryptDecrypt32K 		0x4758
#define AESEncryptDecrypt16K		0x2758

#define EEP_SLAVE_ADDR		0xAE
#define EEP_PAGE_SIZE		64
#define MAX_RETRY_CNT		3
#define MAGIC_NUMBER		0xAA
#define EN_KEY2				0x30

#define F_CPU 				12000000UL  // 12 MHz CPU operation
#define _64KHz				64000UL		// I2C Master clock speed = 64 kHz
#define ACK_bm				0x80
#define NACK_bm				0x40

#define TWI_READ			1
#define TWI_WRITE			0
#define TWI_BAUDRATE(F_SYS, F_TWI)	((F_SYS/(2*F_TWI)) - 5)	// Baudrate formula mentioned in data sheet

// I2C State machine states
typedef enum
{
	I2CMASTER_IDLE = 0,
	WRITE_CHIP_ADDR,
	WRITE_FIRST_ADDR,
	WRITE_SECOND_ADDR,
	READ_EEP_DATA
} I2C_MASTER_STATES;

typedef enum
{
	BL_MODE_REG	= 0,	// Normal/bootloader mode flag
	BL_MAJOR_VER,		// Bootloader major version
	BL_MINOR_VER,		// Bootloader minor version
	FLASH_ADDR_LOW,		// Flash Address Low
	FLASH_ADDR_HIGH,	// Flash Address High
	FLASH_DATA,			// Flash Data
	FLASH_CMD			// Flash Command
} BL_REGS;

typedef enum
{
	STOP = 0,
	START,
	DONE,
	FAILED
} EEP_READ_STATES;

// Mode defs
#define	BOOT_MODE1			1	// Bootloader mode, entered because of missing/bad firmware or forced by user
#define BOOT_MODE2			2	// Bootloader mode, entered because of command
#define BOOT_MODES_gm		0x07
#define ENTER_BL_bm			0x08
#define EXIT_BL_bm			0x10
#define XMEGA32A_bm			0x20
#define MPA3_BATTS			0x40
#define MPA2_BATTS			0x00
#define RD_ONLY_BITS_gm		(~(EXIT_BL_bm | ENTER_BL_bm))

// Flash command defs
#define ERASE_ALL_PAGES_bm	0x08
#define ERASE_FLASE_PAGE_bm	0x04
#define WRITE_FLASH_PAGE_bm	0x02
#define FLASH_BUSY_bm		0x01
#define FLASH_CMDS_gm		(ERASE_ALL_PAGES_bm | ERASE_FLASE_PAGE_bm | WRITE_FLASH_PAGE_bm)

#define FLASH_ADDR_LOW_MAP	0xF0

typedef struct I2C_PARAM_tag
{
	volatile uint8_t  ModeReg;
	uint8_t  MajorVer;
	uint8_t  MinorVer;
	uint16_t FlashAddress;
	uint8_t  DataReg;
	uint8_t  CmdReg;
} I2C_PARAM;

#define NUM_REGS	sizeof(I2C_PARAM)	// Number of registers

typedef struct TWIS_PARAM_TAG
{
	volatile uint8_t State;			// Current state of state machine
	volatile uint8_t ActionReq;
	uint8_t RetryCnt;
	uint8_t ReadExtEEP;
	uint8_t FlashUpdateCnt;
}TWIS_PARAM;


extern TWIS_PARAM	I2C;
extern I2C_PARAM	BL;

// Function prototypes
void Init_I2C(void);
void Deinit_I2C(void);
void Update_I2CStatus(uint8_t state);
void DecryptToasterFirmware(void);
void DoEEPFlashUpdate(void);

#endif	// I2c.h
