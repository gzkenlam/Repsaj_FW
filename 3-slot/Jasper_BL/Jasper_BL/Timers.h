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
// CREATION DATE: 	05/03/2016
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

#define CNT_200MS		205	// Value to be written to RTC period register in order to overflow in 200ms intervals
#define DEF_INTENSITY	20
#define MAX_INTENSITY	99

// Function Prototypes
void SetupSystemClock(void);
void Init_Timers(void);
void Deinit_Timers(void);

#endif 	// Timers.h
