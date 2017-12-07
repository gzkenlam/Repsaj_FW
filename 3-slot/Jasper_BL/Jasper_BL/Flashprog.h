//-----------------------------------------------------------------------------
// FILENAME:  flashprog.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Flash Programming parameters and function prototypes are defined in this file
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
#ifndef _FLASH_PROG_H_
#define _FLASH_PROG_H_

#define MAX_FLASH_UPDATES	2
#define BLANK_DATA			0xFF
#define INVALID_CHECKSUM	0x55

// Flash programming phases
typedef enum
{
	FLASH_PROG_DISABLE = 0,
	READY_TO_WRITE,
	FLASH_PROG_DONE
} FP_STATES;

// Flash programing parameters
typedef struct FLASH_PROG_PARAM_TAG
{
	uint8_t PageBuf[SPM_PAGESIZE];	// define page buffer
	uint16_t BufPtr;
	uint16_t Address;
	uint16_t LastAddress;	
	uint8_t  State;
	uint8_t  UnwrittenData;
	uint8_t EnKey;
} FLASH_PROG_PARAM;

extern FLASH_PROG_PARAM	FP;				// Flash Programing parameters

// Function prototypes
void CheckToasterFirmware(void);
void ClearPageBuffer(void);
void Init_Variables(uint8_t bootmode);
void EraseAllFlashPages(void);
void ErasePage(void);
void WritePage(void);
void WriteUnwrittenPage(uint16_t start);

void nvm_busy_wait(void);
void app_page_erase(uint16_t PageAddr);
void app_page_write(uint16_t PageAddr);

#endif	// FlashProg.h
