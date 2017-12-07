//-----------------------------------------------------------------------------
// FILENAME:  led.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares LED blinking patterns, LED Data structure and function prototypes
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
#ifndef _LED_H_
#define _LED_H_

#include "sysparam.h"

//  LED Pattern bit defs (time is multiples of 100ms ticks)
//  Red/Green/Yellow LED_Patterns
// 	7:4 - Off time 
// 	3:0 - On time

#define ON_TIME(pat)		(pat & 0x0f)
#define OFF_TIME(pat)		(pat >> 4)

#define RED_TEST_PAT	0x77
#define GRN_TEST_PAT	0xEE
#define YEL_TEST_PAT	0x00

#define LED_OFF_PAT		0x00
#define SOLID_ON_PAT	0x01
#define MODERATE_BLINK_PAT	0x55
#define SLOW_BLINK_PAT	0xAA
#define FAST_BLINK_PAT	0x22
#define BLINK_TIME		(ON_TIME(MODERATE_BLINK_PAT) + OFF_TIME(MODERATE_BLINK_PAT))

#define DEF_INTENSITY_AMB_RED	30//85
#define DEF_INTENSITY			8//20
#define MAX_INTENSITY			99
#define BURST_INTENSITY_RED		60//93
#define BURST_INTENSITY_AMB_GRN	30//10
#define BURST_INTENSITY_GRN		80//66
#define BURST_TIME_AMBER		715		// 61ms
#define BURST_TIME_GREEN		1090	// 93ms
#define BURST_REPETITION_TIME	20

#define TEST_PAT_DUR	28	// 2.8 seconds test pattern

typedef enum
{
	CHARGING_DISABLE = 0,
	CHARGING_ON_PROGRESS,
	CHARGING_COMPLETE,
	CHARGING_ERROR,
	BLINK_MODERATE_AMBER,
	BLINK_MODERATE_GREEN,
	BLINK_MODERATE_RED
} CHRG_LED_STATES;

// Data structure in SRAM to hold LED parameters
//	____ __
//		|	  R0
//		|__	  G0
//		|	R1
//	Pat	|__	G1
//		|	  R2
// _____|__   G2
//		|	R0
//		|__	G0
//		|	  R1
//	Cnt	|__   G1
//		|	R2
// _____|__	G2

typedef struct
{
	// LED Colors
	uint8_t Red;
	uint8_t Grn;
} LED_COLORS;

typedef struct
{
	// LED's in all 3 Slots
	LED_COLORS Slot[MAX_SLOTS];
} LED_ALL;

typedef struct LED_PARAM_tag
{
   	LED_ALL	Pat;		// LED Pattern for all 3 Slots
	LED_ALL	Cnt;		// LED Counter for all 3 Slots
	uint8_t TestPatCnt;	// Power on reset Test Pattern Time
	volatile uint8_t *pBestCCReg;
	uint8_t DefIntensityRed;
	uint8_t DefIntensityGrn;
	uint8_t ShowYourSelf;
} LED_PARAM;

#define ALL_LED_COLORS		sizeof(LED_COLORS)
#define ALL_LED_COUNTS		sizeof(LED_ALL)
#define ALL_LED_PATTERNS	sizeof(LED_ALL)

extern LED_PARAM LED;

// Function Prototypes
void Init_LEDs(void);
void Deinit_LEDs(void);
void DoLEDPatterns(void);
void SetChrgLEDPat(uint8_t chrg_state);
void SetShowYourSelf(uint8_t state);

#endif	// LED.h
