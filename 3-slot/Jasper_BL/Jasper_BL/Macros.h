//-----------------------------------------------------------------------------
// FILENAME:  macros.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Bit manipulation macros are defined here
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
#ifndef _MACROS_H_
#define _MACROS_H_

#define TRUE	1
#define FALSE	0

// Macros
#define SetBit(port, bit)		port |= _BV(bit)
#define ClrBit(port, bit)		port &= ~_BV(bit)
#define BitIsSet(port, bit)		(port&_BV(bit))
#define BitIsClr(port, bit)		(!(port&_BV(bit)))

#define NOP					asm("nop")
#define SLEEP1				asm("sleep")

#define CONCAT(a, b)       a##b

#endif // Macros.h
