//-----------------------------------------------------------------------------
// FILENAME:  SysParam.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: System variable definitions and function prototypes are defined in this file
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
#ifndef _SYSPARAM_H_
#define _SYSPARAM_H_

#define F_CPU 			12000000UL  // 12 MHz CPU operation
#define _THERM_

// Comment the following to disable best battery indication
#define _BEST_BATT_
// Comment the following to ENABLE ESD recover count out
//#define _ALWAYS_RECOVER_

#define ENTER_BL_bm		0x08
#define EXIT_BL_bm		0x10
#define RD_ONLY_BITS_gm	(~(EXIT_BL_bm | ENTER_BL_bm))

#define MAX_SLOTS		3
#define SLOT0			0
#define SLOT1			1
#define SLOT2			2

#define MIN_CHRG_CURR		300		// Minimum charging current supported by this hardware

#define MAX_CHRG_CURR		2500	// Maximum charging current supported by this hardware
#define MAX_PWMI_CURR		2490	// The maximum that can be set with n/1500 ratio (Obtained experimentally)
#define PWM_I_MAX			2499	// PWM I positive duty cycle n/1500

#define PWM_V_OFF_VOLT		4200	// Voltage when PWM V is off
#define MIN_CHRG_UP_VOLT	6000	// Minimum charge up voltage supported by this hardware
#define MAX_CHRG_UP_VOLT	10000	// Maximum charge up voltage supported by this hardware
#define VOLT_STEP_SIZE		50		// Charge up voltage can be adjusted by 50mV steps
#define PWM_V_MAX			4095	// PWM V positive duty cycle n/4096

// Active peripheral defs
#define I2C_BATT_bm			0x01
#define A2D_bm				0x08
#define UART_TX_bm			0x10
#define UART_RX_bm			0x20
#define LED_PWM_bm			0x40
#define AUTH_TMR_bm			0x80

#define MAX_I2C_RETRY_CNT	5
#define MAX_COMM_RETRY_CNT	4
#define MAX_BATT_DET_CNT	3  //changed from 8 to 3 for debug
#define BATT_DETECT_TIME	5
#define BATT_INSERT_FLAG	0x01
#define BATT_PRESENT_FLAG	0x10
#define BATT_FLAGS			(BATT_INSERT_FLAG | BATT_PRESENT_FLAG)

// Toaster firmware update through external high capacity EEPROM defs
#define FUB_SLV_ADDR		0xAE
#define FUB_PAGE_SIZE		0x40
#define FUB_BLOCK_SIZE		16
#define ID_BLOCK_ADDR_LO	0xF0
#define ID_BLOCK_ADDR_HI	0x7F
#define MAX_FUB_MEM_ADDR	0x8000
#define OFFSET_DEV_ID		13
#define OFFSET_VER_MAJOR	14
#define OFFSET_VER_MINOR	15
#define OFFSET_CHECKSUM		16

#define SUM_OF_FAST_CHRG_CURR	4500 // Sum of charging currents of all 3 slots at any given time (in mA)

// Register defs
typedef enum
{
	MODE_REG = 0,	// Normal/bootloader mode flag
	FIRM_VER_MAJOR,	// 01-Firmware version, major then minor
	FIRM_VER_MINOR,	// 02
	TOASTER_TYPE	// 03-For Jasper this is fixed to 8
} JASPER_REGS;

// Firmware version showing states
typedef enum
{
	STOP_SHOW_FW_VER = 0,
	START_SHOW_FW_VER,	// 1
	SHOW_MAJOR_FW_VER,	// 2
	SHOW_MINOR_VER_MSD = 4,
	SHOW_MINOR_VER_LSD = 6,
	REPEAT_SHOW_FW_VER = 8
} SHOW_FW_VER_STATES;

// System variables structure
typedef struct SYSTEM_PARAM_tag
{
	uint8_t ModeReg;
	uint8_t RetryCnt;
	uint8_t BattDetCnt;
	uint8_t OneSecCnt;
	uint8_t DebugCnt;
	uint8_t DebugSlot;
	uint8_t BurstRepetitionCnt;
	uint8_t OverCurrRecoveryCnt;
	uint8_t OverCurrRepeatCnt;
	uint8_t OverCurrOneSecCnt;
	volatile uint8_t Actives;
	volatile uint8_t PWMActives;

	volatile uint8_t BattDetect;
	volatile uint8_t NextSlot;
	volatile uint8_t SkipSleep;
	volatile uint8_t ScanSlots;
	uint8_t BestSlot;
	int16_t MaxFastChrgCurr;
	
	uint16_t FUBMemAddr;
	uint8_t FUBDataBuf[FUB_PAGE_SIZE+2];
} SYSTEM_PARAM;

extern SYSTEM_PARAM	SYS;

#endif // SysParam.h
