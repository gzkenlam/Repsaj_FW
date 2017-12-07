//-----------------------------------------------------------------------------
// FILENAME:  pwm.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares PWM V/ PWM I Timer initialization
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "pwm.h"
#include "sysparam.h"
#include "jasper_ports.h"
#include "debug.h"
//----------------------------------------------------------------------
//
// Prototype:	Init_PWM
//
// Description:	Routine to initialize PWM Timers (TC-16 bit)
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Init_PWM(void)
{
	// ------------ Setup PWM V control timer ----------------
	
	TCE0_CTRLB 	= TC0_CCAEN_bm | TC0_CCBEN_bm | TC0_CCCEN_bm | TC_WGMODE_SS_gc;
	TCE0_CNT 	= 0;
	TCE0_PER 	= PWM_V_MAX;

	TCE0_CCABUF = 0;	// PWM_V_0 (PE0)
	TCE0_CCBBUF = 0;	// PWM_V_1 (PE1)
	TCE0_CCCBUF = 0;	// PWM_V_2 (PE2)

	TCE0_CTRLA	= TC_CLKSEL_DIV1_gc;	// Start PWM V Timer (PWM freq = 2.93kHz) 
	
	// -------- Setup PWM ISET1 control timer ---------
	PORTC_REMAP = PORT_TC0A_bm | PORT_TC0B_bm | PORT_TC0C_bm;	// Move TCC0 CCx outputs to higher nibble of PortC
	TCC0_CTRLB 	= TC0_CCAEN_bm | TC0_CCBEN_bm |TC0_CCCEN_bm | TC_WGMODE_SS_gc;
	TCC0_CNT 	= 0;
	TCC0_PER 	= PWM_I_MAX;

	TCC0_CCABUF	= 0;	// PWM_ISET1_0 (PC4)
	TCC0_CCBBUF	= 0;	// PWM_ISET1_1 (PC5)
	TCC0_CCCBUF	= 0;	// PWM_ISET1_2 (PC6)

	TCC0_CTRLA	= TC_CLKSEL_DIV1_gc;	// Start PWM I Timer (PWM freq = 8 kHz)
}

//----------------------------------------------------------------------
//
// Prototype:	Deinit_PWM
//
// Description:	Routine to de-initialize PWM Timers
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Deinit_PWM(void)
{
	TCE0_CTRLA = 0;
	TCC0_CTRLA = 0;

	TCE0_CTRLB = 0;
	TCC0_CTRLB = 0;

	/*
	TCE0_CCABUF = 0;	// PWM_V_0 (PE0)
	TCE0_CCBBUF = 0;	// PWM_V_1 (PE1)
	TCE0_CCCBUF = 0;	// PWM_V_2 (PE2)
	TCE0_CCDBUF = 0;	// PWM_V_3 (PE3)

	TCC0_CCABUF = 0;	// PWM_ISET1_0 (PC4)
	TCC0_CCBBUF = 0;	// PWM_ISET1_1 (PC5)
	TCC0_CCCBUF	= 0;	// PWM_ISET1_2 (PC6)
	TCC0_CCDBUF	= 0;	// PWM_ISET1_3 (PC7)
	*/
}

//----------------------------------------------------------------------
//
// Prototype:	SetPWMV
//
// Description:	Routine to set voltage PWM value for given slot
//
// Parameters:	slot: slot (0 to 3)
//				value: voltage PWM value to be set (from the lockup table)
//
// Returns:		None
// 
// Notes:		None
//----------------------------------------------------------------------
void SetPWMV(uint8_t slot, uint16_t value)
{
	*PWMVReg(slot) =  value;
	SetClrPWMActives(slot, value);	// Add/Clear corresponding voltage PWM to/from active peripherals list
}

//----------------------------------------------------------------------
//
// Prototype:	SetPWMI
//
// Description:	Routine to set current PWM value for given slot
//
// Parameters:	slot: slot (0 to 3)
//				value: current PWM value to be set
//
// Returns:		None
// 
// Notes:		None
//----------------------------------------------------------------------
void SetPWMI(uint8_t slot, uint16_t value) 	
{
	if (value > MAX_PWMI_CURR)
		value = MAX_PWMI_CURR;
	*PWMIReg(slot) = value;
	SetClrPWMActives((slot+4), value);	// Add/Clear corresponding current PWM to/from active peripherals list
}

//----------------------------------------------------------------------
//
// Prototype:	SetClrPWMActives
//
// Description:	Routine to add/clear active PWM timers to/from peripherals list
//
// Parameters:	bitpos: position of the bit to be set/cleared
//				0..3 - PWMI (Slot0 to Slot 3)
//				4..7 - PWMV (Slot0 to Slot 3)
//				value: 0 - Clear, All Other - Set
//
// Returns:		None
// 
// Notes:		None
//----------------------------------------------------------------------
void SetClrPWMActives(uint8_t bitpos, uint16_t value)
{
	uint8_t pwm_active = (1 << bitpos);
	uint8_t sreg = SREG;
	SREG = 0;

	if (!value)
		SYS.PWMActives &= ~pwm_active;
	else
	{
		SLEEP_CTRL	= SLEEP_SEN_bm;			// Set idle mode on sleep
		SYS.PWMActives |= pwm_active;
	}
	SREG = sreg;
}

//----------------------------------------------------------------------
//
// Prototype:	SetActives
//
// Description:	Routine to add new peripheral to active list
//
// Parameters:	active: New peripheral to be added to the active list (refer sysparam.h)
//
// Returns:		None
// 
// Notes:		None
//----------------------------------------------------------------------
void SetActives(uint8_t active)
{
	uint8_t sreg = SREG;
	SREG = 0;							// Disable global interrupts
	SLEEP_CTRL	= SLEEP_SEN_bm;			// Set idle mode on sleep
	SYS.Actives |= active;				// Add new peripheral to the active list
	SREG = sreg;						// Enable global interrupts
}



