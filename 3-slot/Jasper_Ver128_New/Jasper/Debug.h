//-----------------------------------------------------------------------------
// FILENAME:  debug.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Debug commands/ messages and function prototypes are defined in this file
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
#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "sysparam.h"
#define WR_CMD_SIZE	5
#define RD_CMD_SIZE	3
#define RESP_SIZE	3

#define ONE_SEC		10
#define NO_DEBUG	0xFF

// ANSI commands for Hyper terminal
#define CLEAR_SCREEN_CMD	"\x1B[2J"
#define SAVE_CURSOR_CMD		"\x1B[s"
#define RESTORE_CURSOR_CMD	"\x1B[u"

#define TXT_HIGHLIGHT_MSG	"\x1B[31;1m<"
#define TXT_NORMAL_MSG		"\x3e\x1B[30;22m"
#define MSG					"\x1B[10r"

#define MOVE_CURSOR_10_00_MSG	"\x1b[10;00f>"
#define MOVE_CURSOR_03_01_MSG	"\x1b[03;01f"
// Debug messages
#define CMD_PROMPT_MSG		"\r\n>"
#define JASPER_NAME_MSG		"Jasper 3-Slot Charger Ver:"
#define CHECKSUM_MSG		" Checksum:"

#define SLOT_MSG		"Slot :"
#define CHG_DIS_MSG		"Chrg :"
#define CMOD_MSG		"CMOD :"
#define BEST_SLOT_MSG	"BestSlot:"
#define DESIGN_CAP_MSG	"DesignCap:"
#define DBC_CNT_MSG		"DdBatChgCnt:"
#define PWM_V_MSG		"PWM_V:"
#define PWM_I_MSG		"PWM_I:"
#define V_SENSE_MSG		"V_Sen:"

#define BATT_TYPE_MSG	"BattType:"
#define BATT_NONE_MSG	"None"
#define BATT_QLN_MSG	"QLN "
#define BATT_ZQ3_MSG	"ZQ3 "
#define BATT_RLX0_MSG	"RLX0"
#define BATT_RLX1_MSG	"RLX1"
#define BATT_FUB_MSG	"FUB "
#define BATT_DEAD_MSG	"Dead"

#define CC_COUNT_MSG	"CycleCnt:"
#define CC_TRSHLD_MSG	"CCTrshld:"
#define NEXT_MSG		"Next:DynaBlk"

#define CHRG_STATE_MSG	"ChgStat:"
#define CHRG_TIME_MSG	"ChgTime:"
#define CHRG_ERR_MSG	"ChgErr :"
#define FOMT_MSG		"F.O.M.T:"
#define I2C_STATE_MSG	"I2CStat:"
#define I_SENSE_MSG		"I_Sen:"
#define BATT_STATUS_MSG	"Flag:"

#define FAKE_TMP_MSG	"FTmp:"
#define FAKE_SOH_MSG	"FSOH:"
#define VOLT_MSG		"Volt:"
#define CURR_MSG		"Curr:"
#define INST_CURR_MSG	"InstCurr:"
#define ERR_I2C_MSG		"ComErrState:"
#define TEMP_MSG		"Tmp :"
#define SOC_MSG			"SOC :"
#define SOH_MSG			"SOH :"
#define SPACE_MSG		"     "

#define HIGH_MSG		"High"
#define LOW_MSG			"Low "
#define ENABLE_MSG		"Enable "
#define DISABLE_MSG		"Disable"
#define YES_MSG			"Yes"
#define NO_MSG			"No "
#define TAB_MSG			"\t"
#define DIV4096_MSG		"/4096"

#define CRLF_MSG		"\r\n"
#define VOLTS_MSG		"V "
#define AMPS_MSG		"A "
#define CELCIUS_MSG		"C "
#define PERCENT_MSG 	"% "

#define MINS_MSG		" Mins"
#define NA_MSG			"N/A"
#define TAB_MSG			"\t"
#define TWO_TABS_MSG	TAB_MSG TAB_MSG
#define TAB_KEY_MSG		" (TAB)"

#define PORTS_MSG			"\r\nPA PB PC PD PE PR\r\n"
#define FREE_RAM_MSG		"Free RAM:"
#define OVER_CURR_MSG		"OvrCurEvents\r\n"

//Debug commands
#define TAB_KEY		0x09
#define JASPER_NAME		"Jasper 3B Toaster"
#define NO_DEBUG			0xFF
#define DEBUG_CMD_TIMEOUT	1200
#define OVER_CURR_MSG		"OvrCurEvents\r\n"
#define BATT_PRESENT_MSG	"Batt Present:"
// Register defs
typedef enum
{
	RD_MODE_REG = 0,	// Normal/bootloader mode flag
	RD_DATA_REG,
} READ_REGS;

typedef enum
{
	WR_MODE_REG = 0x80,
	WR_SYNC_REG,
} WRITE_REGS;

#define SYNC_BYTE		0xFF
#define ALL_TOASTERS	0x3F
#define WR_CMD_bm		0x80
#define SYNC_LED_NOW	0xFF
#define MAX_SOC			100
#define BATT_SIZE_2X_bm	0x80

#define MFG_ID_SIZE			8
#define PN_SIZE				12
#define DATE_MFG_SIZE		3
#define SN_SIZE				6

typedef struct DEBUG_DATA_tag
{
	uint16_t Volt;
	uint16_t Curr;
	int8_t 	 Tmp;
	uint8_t  SOC;
	uint8_t  SOH;
	uint8_t  ChrgState;
	uint8_t  PartNum[PN_SIZE];
	uint8_t  SerialNum[SN_SIZE];
	uint8_t	 DateMfg[DATE_MFG_SIZE];
	uint8_t  HealthThreshold;
	uint16_t RatedCap;
	uint16_t ChrgCycleCnt;
} DEBUG_DATA;

extern DEBUG_DATA DD[MAX_SLOTS];
extern uint8_t ByteCnt;


// Function prototypes
void Init_Debug(void);
void Init_FreeRAM(void);
void SendByte(uint8_t byte);
void DoDebugCmds(uint8_t recv_byte);
void MoveCursor(uint8_t row, uint8_t col);
void PrintVersionString(void);
void SetupBattChrgData(void);
void PrintBattChrgInfo(void);
void PrintPortValues(void);
void PrintFreeRAM(void);
uint8_t HandleRead(uint8_t data);
void PrintKey(uint8_t *pkey);
void PrintBattEEPData(void);
void PrintBattEEPData2(void);

#endif // debug.h
