//------------------------------------------------------------------------------------
// FILENAME:  ports.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares GPIO initialization routine
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
//------------------------------------------------------------------------------------
#include "jasper_ports.h"

//----------------------------------------------------------------------
//
// Prototype:	SetupPorts
//
// Description:	Routine setup GPIO direction, pull-ups and data registers
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void SetupPorts(void)
{
	// Setup port A
	PORTA_PIN0CTRL = PIN0CTRL_PORTA;
	PORTA_PIN1CTRL = PIN1CTRL_PORTA;
	PORTA_PIN2CTRL = PIN2CTRL_PORTA;
	PORTA_PIN3CTRL = PIN3CTRL_PORTA;
	PORTA_PIN4CTRL = PIN4CTRL_PORTA;
	PORTA_PIN5CTRL = PIN5CTRL_PORTA;
	PORTA_PIN6CTRL = PIN6CTRL_PORTA;
	PORTA_PIN7CTRL = PIN7CTRL_PORTA;

	PORTA_OUT = DATA_PORTA;
	PORTA_DIR = DIR_PORTA;

	// Setup port B
	PORTB_PIN0CTRL = PIN0CTRL_PORTB;
	PORTB_PIN1CTRL = PIN1CTRL_PORTB;
	PORTB_PIN2CTRL = PIN2CTRL_PORTB;
	PORTB_PIN3CTRL = PIN3CTRL_PORTB;

	PORTB_OUT = DATA_PORTB;
	PORTB_DIR = DIR_PORTB;

	// Setup port C
	PORTC_PIN0CTRL = PIN0CTRL_PORTC;
	PORTC_PIN1CTRL = PIN1CTRL_PORTC;
	PORTC_PIN2CTRL = PIN2CTRL_PORTC;
	PORTC_PIN3CTRL = PIN3CTRL_PORTC;
	PORTC_PIN4CTRL = PIN4CTRL_PORTC;
	PORTC_PIN5CTRL = PIN5CTRL_PORTC;
	PORTC_PIN6CTRL = PIN6CTRL_PORTC;
	PORTC_PIN7CTRL = PIN7CTRL_PORTC;

	PORTC_OUT = DATA_PORTC;
	PORTC_DIR = DIR_PORTC;

	// Setup port D
	PORTD_PIN0CTRL = PIN0CTRL_PORTD;
	PORTD_PIN1CTRL = PIN1CTRL_PORTD;
	PORTD_PIN2CTRL = PIN2CTRL_PORTD;
	PORTD_PIN3CTRL = PIN3CTRL_PORTD;
	PORTD_PIN4CTRL = PIN4CTRL_PORTD;
	PORTD_PIN5CTRL = PIN5CTRL_PORTD;
	PORTD_PIN6CTRL = PIN6CTRL_PORTD;
	PORTD_PIN7CTRL = PIN7CTRL_PORTD;

	PORTD_OUT = DATA_PORTD;
	PORTD_DIR = DIR_PORTD;	

	// Setup port E
	PORTE_PIN0CTRL = PIN0CTRL_PORTE;
	PORTE_PIN1CTRL = PIN1CTRL_PORTE;
	PORTE_PIN2CTRL = PIN2CTRL_PORTE;
	PORTE_PIN3CTRL = PIN3CTRL_PORTE;

	PORTE_OUT = DATA_PORTE;
	PORTE_DIR = DIR_PORTE;
	
	// Setup port R
	PORTR_PIN0CTRL = PIN0CTRL_PORTR;
	PORTR_PIN1CTRL = PIN1CTRL_PORTR;

	PORTR_OUT = DATA_PORTR;
	PORTR_DIR = DIR_PORTR;
}

//----------------------------------------------------------------------
//
// Prototype:	Init_ExtInts
//
// Description:	Routine to setup external interrupts
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Init_ExtInts(void)
{
	// Setup over current interrupt to occur at Low level of ICH* pin (PB2)
	PORTB_PIN2CTRL |= PORT_ISC_LEVEL_gc;
	PORTB_INT0MASK  = OVER_CURR_bm;

	// Setup UART receive interrupt to occur at Low level of RXD pin (PC2)
	PORTC_PIN2CTRL |= PORT_ISC_LEVEL_gc;
	PORTC_INT1MASK  = DEBUG_RXD_bm;
}

//----------------------------------------------------------------------
//
// Prototype:	Deinit_ExtInts
//
// Description:	Routine to disable/ clear external interrupts
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Deinit_ExtInts(void)
{
	DISABLE_OVER_CURR_INT;				// Disable over current interrupt
	CLEAR_PENDING_OVER_CURR_INTS;		// Clear pending over current interrupts

	DISABLE_RX_PIN_CHANGE_INT;			// Disable Uart receive pin change interrupt
	CLEAR_PENDING_RX_PIN_CHANGE_INTS;	// Clear pending Uart receive pin change interrupts
}
