//-----------------------------------------------------------------------------
// FILENAME:  timers.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: RTC definitions and function prototypes are defined in this file
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
#ifndef _TIMERS_H_
#define _TIMERS_H_

#define CNT_100MS		102
#define CNT_50MS		51
#define CNT_5MS			5
#define CNT_8MS			8
#define CNT_10MS		10
#define A2D_READ_DELAY	15
#define TEN_TICKS		10
#define I2C_GUARD_TIME	52500	// Corresponding to 280ms (52500 = 280 * 187.5)
#define MICRO_SECS(n)	(3*n)

// Function Prototypes
void SetupSystemClock(void);
void Init_Timers(void);
void Deinit_Timers(void);
void Wait_mSecs(uint8_t wait_time);

#endif 	// Timers.h
