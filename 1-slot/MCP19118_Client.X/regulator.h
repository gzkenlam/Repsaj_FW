/* 
 * File:   regulator.h
 * Author: KLam
 *
 * Created on November 18, 2016, 3:31 PM
 */

#ifndef REGULATOR_H
#define	REGULATOR_H

#include "misc.h"

#ifdef	__cplusplus
extern "C" {
#endif

//Definition for OVCC OVCF
#define IOUT_LSB_ROLL   20
#define MIN_OVCCON  21
#define MAX_OVCCON  85
    
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
    
#define MIN_CALIB_800MV 90     
#define MAX_CALIB_800MV 250
#define MIN_CALIB_800MA 150 
#define MAX_CALIB_800MA 350
#define MIN_CALIB_8V4   1000
#define MAX_CALIB_8V4   1800
#define MIN_CALIB_ZEROA 150
#define MAX_CALIB_ZEROA 350
#define INPUT_THRESHOLD 680
#define THERM_OPEN_ADC  0x0f00
#define THERM_SHORT_ADC 0x0064 

#define CHGSW_On()  do{CHGSW = 1;mcp.u16MCPStatus |= MCP_CHGSW_ON;}while(0)
#define CHGSW_Off() do{CHGSW = 0;mcp.u16MCPStatus &= ~MCP_CHGSW_ON;}while(0)
#define PWMOUT_On()    do{ATSTCON &= ~(_ATSTCON_DRVDIS_MASK); mcp.u16MCPStatus |= MCP_PWMOUT_ON;}while(0)
#define PWMOUT_Off()   do{ATSTCON |= _ATSTCON_DRVDIS_MASK; mcp.u16MCPStatus &= ~MCP_PWMOUT_ON;}while(0)

typedef struct{
    uint16_t    u16MCPStatus;
    uint16_t    u16ErrorStatus;
    uint16_t    u16CurrGoal;
    uint16_t    u16VoltGoal;
    uint8_t     u8OVCC;
    uint8_t     u8OVCF;
    uint8_t     u8Version;
}REGULATOR_VALUE;


extern REGULATOR_VALUE  mcp;
    
void Regulator_Init(void);

void Regulation_Curr(void);

void Regulation_Volt(void);

void MCP_Regulation_Init(void);

void Turn_On_Output(void);

void Turn_Off_Output(void);

void Inc_Iout(void);

void Dec_Iout(void);

void Read_Calib(void);

void Status_Maintain(void);

#ifdef	__cplusplus
}
#endif

#endif	/* REGULATOR_H */

