//-----------------------------------------------------------------------------
// FILENAME:  eeprom.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: EEPROM function prototypes are defined in this file
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

#ifndef _EEPROM_H_
#define _EEPROM_H_

#define INDEX_I_SENSE_CONST		4
#define CALIB_I_OFFSET			16

// function prototypes
uint16_t ReadEEPWord(uint8_t slot);
void WriteEEPData(uint16_t addr, uint8_t size, uint8_t* pdatabuf);
uint8_t GetAuthState(void);

#endif // eeprom.h
