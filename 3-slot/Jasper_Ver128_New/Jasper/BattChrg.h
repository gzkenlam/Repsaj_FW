//-----------------------------------------------------------------------------
// FILENAME:  battchrg.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares battery charging function prototypes are defined here
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

#ifndef _BATTCHRG_H_
#define _BATTCHRG_H_


#define MAX_PWM_V				4064
#define _100mV					100
#define _750mA					750
#define FIG_OF_MERIT_THRESHOLD	80
#define A2D_CALIB_CURR			(int16_t)900
#define INVALID_TMP				(int8_t)0x80
#define DEF_ROOM_TMP			25
#define DEF_HEALTH_THRESHOLD	80
#define DEF_RECHRG_THRESHOLD	89
#define BAD_TMP_VALUE			(int8_t)-17

#define MAX_FAST_CHRG_CURR		2000
#define FAST_CURR_ABOVE_37C		1500
#define FAST_CURR_ABOVE_40C		900
#define FAST_CURR_ABOVE_42C		500

#define _80_SECS			800
#define _10_SECS			100
#define TWO_MINS			120
#define FAST_SETTLE_TIME	50		// Give five seconds to settle avg current
#define SLOW_SETTLE_TIME	50
#define PWMV_SETTLE_TIME	10
#define REMOVAL_WAIT_CNT	4

#define CNT_ONE_SEC				11
#define OVER_CURR_RECOVERY_TIME	50		// Wait 5 seconds before recover from over current condition
#define MAX_OVR_CURR_EVENTS		3

// Misc flags
#define BATT_VERIFIED_bm	0x80
#define MANUAL_PWMI_bm		0x40
#define NEW_FAST_CURR_bm 	0x20
#define FAKE_TMP_bm			0x10
#define FAKE_SOH_bm			0x08
#define FAKE_SOC_bm			0x02
#define FIXED_PWMI_bm		0x01

#define SUPPORTED_DATA_REV	0
#define SUPPORTED_PP_VER	2

#define _200mA				200

typedef enum
{
	DATA_REV = 0,		// 00
	PP_VERSION,			// 01
	DESIGN_CAP,			// 02
	MAX_CHRG_RATE = 4,	// 04
	RECHRG_THRESHOLD = 9,// 09
	CHRG_TMP_LOW,		// 0A
	CHRG_TMP_HIGH = 12,	// 0x0C
	SLOW_TIMEOUT = 18,	// 0x18
	FAST_TIMEOUT,		// 0x13
	PRE_CHRG_VOLT,		// 0x14
	SCL_TEMP1 = 22,
	SCL_RATE1 = 24,
	SCL_VOLT1 = 26,
	SCL_TEMP2 = 28,
	SCL_RATE2 = 30
} AUTH__DATA_SLOT8;

typedef enum
{
	SCL_VOLT2 = 0,
	SCL_TEMP3 = 2,
	SCL_RATE3 = 4,
	SCL_VOLT3 = 6,
	NO_BATT_TEMP = 8,
	CHRG_TERM_CURR = 10
} AUTH__DATA_SLOT9;

typedef enum
{// NOTE: Don't change the following order
	BATT_TYPE_NONE = 0,
	BATT_TYPE_DEAD,	// 1
	BATT_TYPE_QLN,	// 2
	BATT_TYPE_ZQ3,	// 3
	BATT_TYPE_RLX0,	// 4	// Rolex batteries that doesn't have slot8 data
	BATT_TYPE_RLX1,	// 5
	BATT_TYPE_FUB	// 6 	
} BATT_TYPES;

typedef enum
{
	REQUEST_TO_CHRG = 0,
	TEST_PWM_V_CIRCUIT,	// 01
	SET_SLOW_CHRG_PWM,	// 02
	START_SLOW_CHRG,	// 03
	DO_SLOW_CHRG,		// 04
	SET_FAST_CHRG_PWM,	// 05
	START_FAST_CHRG,	// 06
	DO_FAST_CHRG,		// 07
	CHRG_NEARLY_DONE,	// 08
	FULLY_CHARGED,		// 09
	CHRG_PAUSE,			// 0A
	BAD_TMP				// 0B
} CHRG_STATES;

typedef enum
{
	// Battery found errors
	NO_ERROR = 0,
	BAD_DATA_CKSUM,		// 01
	BAD_DATA_TYPE,		// 02
	BAD_SLOW_CURR,		// 03
	BAD_FAST_CURR,		// 04
	BAD_CHRG_UP_V,		// 05
	
	BATT_ID_MISMATCH,	// 06	Mfg ID in EEPROM are not matched
	BATT_UNSUPPORTED,	// 07	This battery is neither Amigos nor FUB
	
	// Charger errors
	ABNORMAL_CURR_FAULT = 0x20,
	PWM_V_FAULT,		// 21
	SLOW_TIME_FAULT,	// 22
	SLOW_CURR_FAULT,	// 23
	FAST_TIME_FAULT,	// 24
	FAST_CURR_FAULT,	// 25
	FUEL_GAUGE_FAULT,	// 26
	OVER_CURR_FAULT,	// 27
	
	BATT_COMM_FAULT,	// 28
	
	// Battery authentication errors
	AUTH_ERR_READ_SN = 0x30,	// 30
	AUTH_ERR_VERIFY_SN,			// 31
	AUTH_ERR_READ_SLOT_DATA,	// 32
	AUTH_ERR_GEN_RND_NUM,		// 32
	AUTH_ERR_GEN_DIGEST,		// 34
	AUTH_ERR_NONCE_MLB,			// 35
	AUTH_ERR_DIGEST_RAD_NUM,	// 36
	AUTH_ERR_DRIVE_KEY,			// 37
	AUTH_ERR_READ_SLOT4,		// 38
	AUTH_ERR_WRITE_SLOT,		// 39
	
	AUTH_ERR_CMD_SHA256,		// 3A
	AUTH_ERR_VERIFY_SIG_CERT,	// 3B
	AUTH_ERR_VERIFY_SIG_CERT2,	// 3C
	AUTH_ERR_READ_SLOT8,		// 3D
	INVALID_BATT_DATA_REV,		// 3E
	INVALID_PP_VERSION,			// 3F
} CHRG_ERRORS;

//------- Dead Battery charging parameters --------------------
#define DEAD_SLOW_TIMEOUT		60
#define DEAD_FAST_TIMEOUT		450

#define DEAD_CHRG_UP_VOLT		8400
#define DEAD_SLOW_FAST_VOLT		5600	// Note: This must be higher than BAT_LOW_LEVEL
#define DEAD_RECHRG_THRESHOLD	89

#define DEAD_ABNORMAL_CURR		1500
#define DEAD_FAST_CHRG_CURR		1000
#define DEAD_MAX_SLOW_CURR		200
#define DEAD_NEARLY_DONE_CURR	200
#define DEAD_DONE_CURR			150

#define DEAD_COLD_OFF_TMP		-2
#define DEAD_COLD_ON_TMP		0
#define DEAD_HOT_ON_TMP			43
#define DEAD_HOT_OFF_TMP		45
//-------------------------------------------------------------


// Function prototypes
void Init_BattCharger(void);
uint8_t DoCharger(void);
void ShutdownCharger(void);
void DoBattChrg(uint8_t next_i2c_state);
void ScanChrgSlots(void);
void HandleOverCurrInt(void);
void AutoAdjustPWMI(int16_t set_current);
void FindBestBattSlot(void);
void SetChrgupVoltage(void);

#endif // BattChrg.h
