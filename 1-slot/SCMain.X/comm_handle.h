/* 
 * File:   uart_handle.h
 * Author: KLam
 *
 * Created on November 23, 2016, 6:22 PM
 */

#ifndef COMM_HANDLE_H
#define	COMM_HANDLE_H

#include "global.h"
#include <string.h>
#include <stdio.h>

#ifdef	__cplusplus
extern "C" {
#endif

//#define EUSART_DataReady  (eusartRxCount)

// ANSI commands for Hyper terminal
#define CLEAR_SCREEN_CMD	"\x1B[2J"
#define SAVE_CURSOR_CMD		"\x1B[s"
#define RESTORE_CURSOR_CMD	"\x1B[u"

#define TXT_HIGHLIGHT_MSG	"\x1B[31;1m<"
#define TXT_NORMAL_MSG		"\x3e\x1B[30;22m"
#define MSG					"\x1B[10r"

#define MOVE_CURSOR_13_00_MSG	"\x1b[13;00f>"
#define MOVE_CURSOR_01_01_MSG	"\x1b[01;01f"
#define MOVE_CURSOR_03_01_MSG	"\x1b[03;01f"
#define MOVE_CURSOR_06_01_MSG	"\x1b[06;01f"
#define MOVE_CURSOR_09_01_MSG	"\x1b[09;01f"
// Debug messages
#define CMD_PROMPT_MSG		"\r\n>"
#define AMIGOS_NAME_MSG		"Jasper 1-Slot Charger Ver:"
#define CHECKSUM_MSG		"Checksum:"

#define SLOT_MSG		"Slot :"
#define CHG_DIS_MSG		"Chrg :"
#define BEST_SLOT_MSG	"BestSlot:"
#define DESIGN_CAP_MSG	"DesignCap:"
#define PWM_V_MSG		"PWM_V:"
#define PWM_I_MSG		"PWM_I:"
#define V_SENSE_MSG		"V_Sen:"

#define CC_COUNT_MSG	"CycleCnt:"
#define CCA_TIME_MSG	"CCATime:"
#define NEXT_MSG		"Next:DynaBlk"

#define CHRG_STATE_MSG	"ChgStat:"
#define CHRG_TIME_MSG	"ChgTime:"
#define CHRG_ERR_MSG	"ChgErr :"
#define FOMT_MSG		"F.O.M.T:"
#define I2C_STATE_MSG	"I2CStat:"
#define I_SENSE_MSG		"I_Sen:"

#define FAKE_TMP_MSG	"FTmp:"
#define FAKE_SOH_MSG	"FSOH:"
#define VOLT_MSG		"Volt:"
#define CURR_MSG		"Curr:"
#define TEMP_MSG		"Tmp :"
#define SOC_MSG			"SOC :"
#define SOH_MSG			"SOH :"
#define SPACE_MSG		"     "

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
#define TWO_TABS_MSG	TAB_MSG TAB_MSG
#define TAB_KEY_MSG		" (TAB)"

#define PORTS_MSG			"\r\nPA PB PC PD PE PR\r\n"
#define FREE_RAM_MSG		"Free RAM:"
#define OVER_CURR_MSG		"OvrCurEvents\r\n"
    
#define I2C_Address_MCP19118        0x15
#define I2C_Address_FuelGauge_1     0x0B
#define I2C_Address_FuelGauge_2     0x55
#define I2C_Address_ATECC_HOST      0x60
#define I2C_Address_ATECC_DEVICE    0x61    //ZQ3
//#define I2C_Address_ATECC_DEVICE    0x62    //Rolex

#define MCP19118_ADDRESS    (I2C_Address_MCP19118)
#define FUELGAUGE_1_ADDRESS (I2C_Address_FuelGauge_1)
#define FUELGAUGE_2_ADDRESS (I2C_Address_FuelGauge_2)
#define CLIENT_ADDRESS      (I2C_Address_ATECC_DEVICE)
#define MLB_ADDRESS         (I2C_Address_ATECC_HOST)
    
    



    
/* Debug message type */    
#define DEBUG_MCP_STATUS        0x01
#define DEBUG_FUELGAUGE_STATUS  0x02
#define DEBUG_ALL_STATUS        0x03
#define DEBUG_CHARGE_STATUS     0x04
#define DEBUG_MESS_OFF          0x00
    
//Definition for MCPStatus
#define MCP_CALIBED     0x0001
#define MCP_INIT        0x0002
#define MCP_CURR_REG    0x0004
#define MCP_VOLT_REG    0x0008
#define MCP_CURR_STABLE 0x0010
#define MCP_VOLT_STABLE 0x0020
#define MCP_PWMOUT_ON   0x0040
#define MCP_CHGSW_ON    0x0080
#define MCP_LOAD_DET    0x0100
#define MCP_BATT_DET    0x0200
#define MCP_DETECT_ON   0x0400
#define MCP_GETCMD      0x2000
#define MCP_REMOTE      0x4000
#define MCP_ERROR       0x8000
    
//Definition for ErrorStatus
#define INUVP           0x0001
#define INOVP           0x0002
#define OUTOVP          0x0004
#define OUTOCP          0x0008
#define OUTUVP          0x0010
#define THERM_OPEN      0x0020
#define THERM_SHORT     0x0040
#define THERM_OVER      0x0080      //set by main controller
#define SLOW_TIME_OUT   0x0100      //set by main controller
#define FAST_TIME_OUT   0x0200      //set by main controller
    
typedef struct{
    uint16_t    u16MCPStatus;
    uint16_t    u16ErrorStatus;
    uint16_t    u16CurrGoal;
    uint16_t    u16VoltGoal;
    uint8_t     u8OVCC;
    uint8_t     u8OVCF;
    uint8_t     u8Version;
    uint16_t    u16Vin;
    uint16_t    u16VBat;
    uint16_t    u16IOut;
    uint16_t    u16Therm;
    uint16_t    u16Calib800mA;
    uint16_t    u16CalibZeroA;
    uint16_t    u16Calib800mV;
    uint16_t    u16Calib8V4;
}MCP_DATA_TYPE;
extern MCP_DATA_TYPE mcpdata;

/* command list of MCP19118 */    
#define NONE_CMD        0x00
#define INIT_CMD        0x01
#define PWM_ON          0x02
#define PWM_OFF         0x03
#define CHGSW_ON        0x04
#define CHGSW_OFF       0x05
#define TURN_ON         0x06
#define TURN_OFF        0x07
#define DEC_CURR        0x08
#define INC_CURR        0x09
#define VOLT_REG_ON     0x0a
#define VOLT_REG_OFF    0x0b
#define CURR_REG_ON     0x0c
#define CURR_REG_OFF    0x0d
#define CURR_REG_OUT    0x0e
#define VOLT_REG_OUT    0x0f
#define SET_ERROR       0x10
#define SET_STATUS      0x11
#define SET_CALIB       0x12
#define CLR_CALIB       0x13
#define CALIB_800MA     0x14
#define CALIB_800MV     0x15
#define CALIB_8V4       0x16
#define CALIB_ZEROA     0x17
#define WR_CALIB        0x18
#define RD_CALIB        0x19
#define SET_REMOTE      0x1a
#define CLR_REMOTE      0x1b
#define SET_THERM_ERR   0x1c
#define CLR_THERM_ERR   0x1d
#define SET_TIMER_ERR   0x1e
#define DEC_HIGH_CURR   0x28
#define INC_HIGH_CURR   0x29

typedef struct{
    uint8_t     u8SendCmd;
    uint16_t    u16SetVal1;
    uint16_t    u16SetVal2;
}CMD_DATA_TYPE;
extern CMD_DATA_TYPE cmd;

/* Command for Fuel Gauge Legacy */
#define FG_TYPE_AMOUNT  2

#define FULL_CAP_1    0x18
#define S_O_C_1       0x0d
#define CYCLE_COUNT_1 0x17
#define BATT_VOL_1    0x09
#define BATT_CURR_1   0x0a
#define TEMPERATURE_1 0x08
#define BATT_STATUS_1 0x16
#define S_O_H_1       0x4f
#define BATT_MODE_1   0x03
#define MAC_1         0x44

/* Command for Fuel Gauge new */
#define FULL_CAP_2    0x12
#define S_O_C_2       0x2C
#define CYCLE_COUNT_2 0x2A
#define BATT_VOL_2    0x08
#define BATT_CURR_2   0x0C
#define TEMPERATURE_2 0x06
#define BATT_STATUS_2 0x0a
#define S_O_H_2       0x2e
#define BATT_MODE_2   0x0a      //Dummy for ZQ3, as it is not support
#define MAC_2         0x3E

/* Definition of battdata.u8Status*/
#define ECC508_VALID        0x01
#define ECC508_NOT_VALID    0x02
#define OTHER_BATT          0x03

/* structure for fuelgauge data */
typedef struct{
    uint8_t u8Status;
    uint8_t u8FgAddr;
    uint16_t    u16Capacity;
    uint16_t    u16SOC;
    uint16_t    u16Cycle;
    int16_t    s16Vol;
    int16_t   s16Cur;
    uint16_t    u16TempK;
    uint16_t    u16Status;
    uint16_t    u16SOH;
    //uint16_t    u16BattMode;
}BATT_DATA_TYPE;
extern  BATT_DATA_TYPE battdata;

typedef struct{
    uint8_t     u8DataRev;
    uint8_t     u8PPRev;
    uint16_t    u16DesignCapacity;
    uint16_t    u16MaxCurr;
    uint8_t     u8Faire;
    uint8_t     u8Unfaire;
    uint8_t     u8Low;
    uint8_t     u8RechargeThreshold;
    uint16_t    u16ChargeLowTempK;
    uint16_t    u16ChargeHighTempK;
    uint16_t    u16DischargeHighTempK;
    uint16_t    u16DischargeLowTempK;
    uint8_t     u8SlowChargeTimeout;
    uint8_t     u8FastChargeTimeout;
    uint16_t    u16PrechargeThreshold;
    uint16_t    u16SCLTempK1;
    uint16_t    u16SCLRate1;
    uint16_t    u16SCLVolt1;
    uint16_t    u16SCLTempK2;
    uint16_t    u16SCLRate2;
    uint16_t    u16SCLVolt2;
    uint16_t    u16SCLTempK3;
    uint16_t    u16SCLRate3;
    uint16_t    u16SCLVolt3;
    uint16_t    u16NoBattTempK;
    uint8_t     u8ChargeTC;
}CHARGE_DATA_TYPE;
extern  CHARGE_DATA_TYPE battspec;

typedef struct{
    uint8_t u8REV;
    uint8_t u8DTL;
    uint16_t u16RSVD;
}SLOT7_DATA_TYPE;
extern SLOT7_DATA_TYPE battslot7;

#define DEFAULT_CYCLE_OVER  600
#define DEFAULT_CYCLE_LIMIT 550
#define DEFAULT_SOH_LIMIT 79

#define FG_HIGH_TEMPK    (3180)  //45C
#define FG_LOW_TEMPK     (2730)  //0C
#define FG_FAST_TIMEOUT  (8)  //60*60*8
#define FG_SLOW_TIMEOUT  (1)   //60*60*1
#define FG_CC_CURRMA     (1000)   //1000mA
#define FG_CV_VOLTMV     (8400)
#define FG_TC_CURRMA     (150)  //150mA
#define FG_RC_THRESHOLD  (89)   //89%
#define FG_PC_THRESHOLD  (5600) //5.6V

#define DEFAULT_CALIB_800mV (133)       
#define DEFAULT_CALIB_8V4   (1398)
#define DEFAULT_CALIB_800mA (228)

extern char cOutString[];    
extern uint8_t u8Unhealth;
extern uint8_t u8DebugOn;

void Buffer_Init(void);
void USART_Handle(void);
uint8_t MCP_CmdSend(void);
//bool MCP_DataRead(void);
void Debug_Display(void);
void Show_Version(void);
void Show_SampleData(void);
void Debug_Mode(uint8_t disp);
void Batt_Comm(void);
void BattDetect_DeJitter(void);


#ifdef	__cplusplus
}
#endif

#endif	/* UART_HANDLE_H */

