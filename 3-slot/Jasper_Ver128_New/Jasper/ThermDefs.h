//-----------------------------------------------------------------------------
// FILENAME:  thermdefs.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Battery Thermistor definitions are here
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

#ifndef _THERM_DEFS_
#define _THERM_DEFS_

#define THERM_DATA_ENTRIES		9
#define THERM_DATA_SIZE 		28
#define R1 						165
#define R2						249

#define EQU_RES					((uint32_t)R1*R2*1000)/(R1+R2)
#define K						1970	//=(A2D_RES * Vref_Divider * Alpha), where Alpha = R2/(R1+R2), Vref_Divider = 1.6, A2D_RES = 2047

//--------- Jasper battery Thermistor Data -----------
typedef enum
{
	TMP0 = -9,
	TMP1 = -1,
	TMP2 = 7,
	TMP3 = 15,
	TMP4 = 23,
	TMP5 = 31,
	TMP6 = 39,
	TMP7 = 47,
	TMP8 = 55
} TMP_ENTRIES;

typedef enum
{
	RES0 = 0x9C78,
	RES1 = 0xC2D7,
	RES2 = 0xF846,
	RES3 = 0xA1F6,
	RES4 = 0xD7A5,
	RES5 = 0x9275,
	RES6 = 0xCAE4,
	RES7 = 0x8F04,
	RES8 = 0xCD33
} RES_ENTRIES;

#define TMP_CORR_FACTOR		0x0C
//--------------------------------------------------

#endif // ThermDefs.h
