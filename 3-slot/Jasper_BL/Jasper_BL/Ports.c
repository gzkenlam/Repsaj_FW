//------------------------------------------------------------------------------------
// FILENAME:  ports.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares GPIO intialization routine
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
//------------------------------------------------------------------------------------
#include "Jasper_ports.h"

//------------------------------------------------------------------------------------
//
// Prototype: 	SetupPorts(void)
//
// Description:	Routine to setup GPIO ports, there directions and pull up configurations
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//------------------------------------------------------------------------------------
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
}


