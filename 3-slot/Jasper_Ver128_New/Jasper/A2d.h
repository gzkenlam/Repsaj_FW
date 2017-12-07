//-----------------------------------------------------------------------------
// FILENAME:  a2d.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: A2d data structures and function prototypes are defined in this file
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
#ifndef _A2D_H_
#define _A2D_H_

#include "sysparam.h"

#define CUR_BUF	0
#define TMP_BUF	1

#define A2D_RES			2047
#define MAX_SUM_COUNT	8

#define MAX_GAIN_ERROR	15		// About 2 LSBs

#define BAT_DETECT_HI	9700	// Battery detection voltages (100mV < < 9.7V)
#define BAT_DETECT_LO	100

#define BAT_REMOVE_HI	10700	// Battery remove voltage high, if fuse FET is on
#define BAT_REMOVE_LO	50		// Battery remove voltage low, if fuse FET is off

#define FUSE_OFF_RAMP	175		// mV/100mS
#define FUSE_ON_RAMP	500		// mV/100mS

#define BAT_LOW_LEVEL	5400

#define GetA2DVal(slot)		(int16_t)(*(&ADCA.CH0RES + slot))
#define DEF_V_SENSE_CONST	12354//11646	// The default value
#define DEF_I_SENSE_CONST	33000

typedef struct A2D_P_tag
{
	uint16_t ISenseConst;
	uint16_t VSenseConst;
	int16_t BattVSense;
	int16_t Sum;
} A2D_P;

typedef struct A2D_PARAM_tag
{
	A2D_P	Slot[MAX_SLOTS];
	int16_t BattISense;
	int16_t BattTSense;
	uint8_t AvgCnt;
} A2D_PARAM;

extern A2D_PARAM A2D;

// Function Prototypes
void Init_A2D(void);
void Deinit_A2D(void);
uint8_t ReadCalibrationByte(uint8_t index);
uint8_t SetA2DMuxForCurrTmp(void);
uint8_t ReadA2DCurrTmp(uint8_t next_i2c_state);

#endif /* a2d.h */
