//-----------------------------------------------------------------------------
// FILENAME:  Batt_Defs.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Battery data definitions are here
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

#ifndef _BATT_DEFS_H_
#define _BATT_DEFS_H_

#include "SysParam.h"
#include "BattAuth.h"

typedef struct BATT_DATA_tag
{
	uint8_t I2CState;
	volatile uint8_t I2CBusy;
	uint8_t MiscFlags;
	int8_t  Tmp;
	uint8_t SOC;
	uint8_t SOH;
	uint8_t WaitCnt;
	uint8_t CommRetryCnt;
	uint8_t Fault;
	uint8_t BattType;
	uint8_t DeadBattChrgCnt;

	//uint8_t FOMThreshold;
	uint16_t DesignCap;
	
	uint8_t  ChrgState;	
	uint16_t FastChrgCurr;
	uint8_t  AvgCurrSettleCnt;
	uint8_t  PWMSettleCnt;
	uint16_t ChrgTimeSecs;
	uint16_t FullyChrgedCnt;
	uint8_t  NearlyChrgedCnt;
	uint16_t ChrgCycleCount;	
	int16_t  Volt_PWM_Off;
	uint16_t MaxPWMV;
	uint8_t CommErrI2CState;
	
	uint8_t FWMajorVer;
	uint8_t FWMinorVer;

} BATT_DATA;

#define BLOCK_DATA_SIZE 	32
#define LOW_BYTE			0
#define HIGH_BYTE			1

typedef struct GAS_GAUGE_DATA_tag
{
	//NOTE: Don't change the order of following variables
	uint16_t TmpKelvin;
	uint16_t Volt;
	
	int16_t AvgCurr;
	int16_t GGInstCurr;	// Read by gg
	int16_t InstCurr;	// Read by A2D
	
	uint8_t Status[2];

} GAS_GAUGE_DATA;

extern GAS_GAUGE_DATA Gauge;
extern uint8_t CommBuf[MAX_AUTH_CMD_SIZE];

extern BATT_DATA Batt[MAX_SLOTS];
extern BATT_DATA *pBatt;

#endif	// batt_defs.h
