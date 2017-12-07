//-----------------------------------------------------------------------------
// FILENAME:  battfound.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: battery communication function prototypes are defined in this file
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
#ifndef _BATTFOUND_H_
#define _BATTFOUND_H_

// Function prototypes
uint8_t GetBlockCksum(uint8_t block_num);
uint8_t CheckDynaBlock(uint8_t dyna_block);
uint8_t CheckGGMfgBlock(const char *pstr);
uint8_t CheckBattFound(void);
void SetBatteryPresetFlag(uint8_t batt_type);
void ShowFWVersion(void);

extern uint8_t Cmd;

#endif	// battfound.h
