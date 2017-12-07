//-----------------------------------------------------------------------------
// FILENAME:  pp_battdefs.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: PP battery definition are here
//
// %IF Zebra_Internal
//
// NOTES:
//
// AUTHOR:   		Wasath Mudalige
// CREATION DATE: 	04/25/2016
// DERIVED FROM: 	New File
//
// EDIT HISTORY:
//
//
//
// %End
//-----------------------------------------------------------------------------


#ifndef PP_BATTDEFS_H_
#define PP_BATTDEFS_H_

#define GG_SLV_ADDR_QLN		0x16
#define GG_SLV_ADDR_ZQ3		0xAA

#define DEF_CC_THRESHOLD	550
#define KELVIN_CONST		273

#define COMMON_DEV_NAME		"ZBM"
#define COMMON_DEV_NAME_SIZE	3

#define ROLEX_DEV_NAME		"P1089503"
#define QLN_DEV_NAME		"ZBM"

typedef enum
{
	MFG_ACCESS	= 0,	// 0x00
	REM_CAP_ALARM,		// 0x01
	REM_TIME_ALARM,		// 0x02
	BATT_MODE,			// 0x03
	AT_RATE,			// 0x04
	AT_RATE_TIME_TO_FULL,//0x05
	AT_RATE_TIME_TO_EMPT,//0x06
	AT_RATE_OK,			// 0x07
	TEMPERATURE,		// 0x08
	VOLTAGE,			// 0x09
	INST_CURRENT,		// 0x0a
	AVG_CURRENT,		// 0x0b
	MAX_ERROR,			// 0x0c
	REL_STATE_OF_CHRG,	// 0x0d
	ABS_STATE_OF_CHRG,	// 0x0e
	REMAINING_CAP,		// 0x0f
	FULL_CHRG_CAP,		// 0x10
	RUN_TIME_TO_EMPTY,	// 0x11
	AVG_TIME_TO_EMPTY,	// 0x12
	AVG_TIME_TO_FULL,	// 0x13
	CHARGING_CURRENTS,	// 0x14
	CHARGING_VOLTAGE,	// 0x15
	BATT_STATUS ,		// 0x16
	CYCLE_COUNT,		// 0x17
	DESIGN_CAP_QLN,		// 0x18
	DESIGN_VOLTAGE,		// 0x19
	SPEC_INFO,			// 0x1a
	MFG_DATE,			// 0x1b
	SERIAL_NUM,			// 0x1c
	
	MFG_NAME = 0x20,	// 0x20
	DEVICE_NAME,		// 0x21
	DEVICE_CHEMISTRY,	// 0x22
	MFG_DATA,			// 0x23
	
	CC_THRESHOLD = 0x28,// 0x28
	CAP_RATIO,			// 0x29
	SOC_THERSHOLD,		// 0x2a
	LED_BLINK_RATE,		// 0x2b
	LED_ON_TIME,		// 0x2c
	LED_OFF_TIME,		// 0x2d
	LED_DISABLE,		// 0x2e
	AUTHENTICATION,		// 0x2f
	SHIP_MODE_CMD,		// 0x30
	
	DATE_FIRST_USED = 0x38,	// 0x38
	
	SUP_CELL_MODEL = 0x3C,// 0x3c
	BARCODE,			// 0x3d
	MODEL_NUMBER,		// 0x3e
	
	//--------------------
	MFG_BLOCK_ACCESS = 0x44,
	STATE_OF_HEALTH	= 0x4F
	//--------------------
} QLN_GG_CMDS;

typedef enum
{
	BATT_TEMP_ZQ3	= 0x06,
	BATT_VOLT_ZQ3	= 0x08,
	BATT_STATUS_ZQ3 = 0x0A,
	INST_CURR_ZQ3	= 0x0C,
	REM_CAP_ZQ3		= 0x10,
	FULL_CHRG_CAP_ZQ3 = 0x12,
	AVG_CURR_ZQ3	= 0x14,
	TIME_TO_EMPTY_ZQ3 = 0x16,
	TIME_TO_FULL_ZQ3 = 0x18,
	STANDBY_CURR_ZQ3 = 0x1A,
	CYCLE_CNT_ZQ3	= 0x2A,
	REL_SOC_ZQ3		= 0x2C,
	BATT_SOH_ZQ3	= 0x2E,
} ZQ3_GG_CMDS;

#define FC_bm	0x20		// Fully charged bit in status flag

typedef struct EEP_CHRG_DATA_tag
{
	uint8_t  SlowChrgMins;
	uint16_t FastChrgMins;
	uint16_t SlowFastVolt;
	uint8_t RechrgThreshold;
	
	int16_t AbnormalChrgCurr;
	int16_t SlowChrgCurr;
	int16_t NearlyDoneCurr;
	int16_t ChrgTermCurr;

	int8_t   ColdOff;
	int8_t   ColdOn;
	int8_t   HotOn;
	int8_t   HotOff;
	
	uint16_t CCThreshold;
	uint8_t HealthThreshold;
	
	uint16_t ChrgupVolt;
	int16_t FastChrgCurr;
	
	// Range 1
	int8_t MaxTemp1;
	int16_t FastChrgCurr1;
	uint16_t ChrgupVolt1;
	
	// Range 2
	int8_t MaxTemp2;
	int16_t FastChrgCurr2;
	uint16_t ChrgupVolt2;
	
	// Range 3
	int8_t MaxTemp3;
	int16_t FastChrgCurr3;
	uint16_t ChrgupVolt3;

} EEP_CHRG_DATA;

extern EEP_CHRG_DATA EEP[MAX_SLOTS];
extern EEP_CHRG_DATA *pEEP;


#endif /* PP_BATTDEFS_H_ */