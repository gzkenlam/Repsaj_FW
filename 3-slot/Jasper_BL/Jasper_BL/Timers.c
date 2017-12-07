//-----------------------------------------------------------------------------
// FILENAME:  timers.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares Timer/ clock initialization functions and ISR
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include "i2c.h"
#include "uart.h"
#include "macros.h"
#include "timers.h"
#include "flashprog.h"
#include "Jasper_ports.h"

//----------------------------------------------------------------------
//
// Prototype:	SetupSystemClock(void)
//
// Description:	Routine to setup system clock to 12MHz using internal PLL
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void SetupSystemClock(void)
{
	// Check for 2MHz osc stable
	while (!(OSC_STATUS & OSC_RC2MRDY_bm));

	// Set prescaler A & B to /1, C to /2
	CPU_CCP = CCP_IOREG_gc;		// Allow config change
	CLK_PSCTRL = CLK_PSBCDIV0_bm;

	// Set the PLL to 2Mhz int osc as source and multiply by 12
	OSC_PLLCTRL = OSC_PLLFAC3_bm | OSC_PLLFAC2_bm;

	// Enable the PLL
	OSC_CTRL |= OSC_PLLEN_bm;

	// Check for PLL stable
	while (!(OSC_STATUS & OSC_PLLRDY_bm));

	// Switch to PLL for system clock
	CPU_CCP = CCP_IOREG_gc;
	CLK_CTRL = CLK_SCLKSEL2_bm;
}
//----------------------------------------------------------------------
//
// Prototype:	Init_Timers(void)
//
// Description:	Routine to initialize RTC Timer
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Init_Timers(void)
{
	// Setup RTC to interrupt every 200ms (Blink LED's)
	OSC_CTRL |= OSC_RC32KEN_bm;				// Enable 32.768kHz Internal RC oscillator
	while (!(OSC_STATUS & OSC_RC32KRDY_bm));// Wait until 32.768kHz is stabilized

	CLK_RTCCTRL = CLK_RTCSRC_RCOSC_gc | CLK_RTCEN_bm;	// Setup RTC Clock source as 1.024kHz from 32.768kHz internal RC oscillator

	RTC_CNT		= 0; 						// Clear counter value
	while(RTC_STATUS & RTC_SYNCBUSY_bm);	// Wait till counter sync with sub clock

	RTC_PER	= CNT_200MS;					// Set period such that overflow interrupt occurs every 200ms
	while(RTC_STATUS & RTC_SYNCBUSY_bm);	// Wait till period sync with sub clock
	
	RTC_INTFLAGS= RTC_COMPIF_bm | RTC_OVFIF_bm;	// Clear any pending RTC interrupts
	RTC_INTCTRL	= RTC_OVFINTLVL_LO_gc;		// Interrupt level Low
	RTC_CTRL	= RTC_PRESCALER0_bm;		// Start RTC, Set prescale /1
	
	TCD2_CTRLA = 0x00;
	TCD2_CTRLB = 0x00;
	TCD2_CTRLC = 0x00;
	TCD2_CTRLE = TC2_BYTEM_SPLITMODE_gc;
	TCD2_INTCTRLA = 0x00;
	TCD2_INTCTRLB = 0x00;
	
	TCD2_HCNT = 0;
	TCD2_LCNT = 0;
	
	TCD2_HPER = MAX_INTENSITY;
	TCD2_LPER = MAX_INTENSITY;
	
	TCD2_LCMPA = DEF_INTENSITY;		// Red0	- 928
	TCD2_HCMPA = DEF_INTENSITY;		// Red2 - 929
	
	TCD2_LCMPB = DEF_INTENSITY;		// Grn0	- 92A
	TCD2_HCMPB = DEF_INTENSITY;		// Grn2	- 92B
	
	TCD2_LCMPC = DEF_INTENSITY;		// Red1	- 92C
	TCD2_HCMPC = DEF_INTENSITY;		// Red3 - 92D
	
	TCD2_LCMPD = DEF_INTENSITY;		// Grn1	- 92E
	TCD2_HCMPD = DEF_INTENSITY;		// Grn3	- 92F
	
	/*
	TCD2_LCNT = 0;
	TCD2_LPER = MAX_INTENSITY;
	
	TCD2_LCMPA = DEF_INTENSITY;	// Red0	- 928
	TCD2_LCMPB = DEF_INTENSITY;	// Grn0	- 92A
	*/
	TCD2_CTRLA = TC_CLKSEL_DIV4_gc;		// Start LED PWM Timer at 30kHz
}

//----------------------------------------------------------------------
//
// Prototype:	Deinit_Timers
//
// Description:	Routine to de-initialize Timers
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Deinit_Timers(void)
{
	RTC.CTRL = 0;
	RTC_INTCTRL	= 0;
	RTC_INTFLAGS= RTC_COMPIF_bm | RTC_OVFIF_bm;	// Clear any pending RTC interrupts

	TCD2_CTRLA = TC_CLKSEL_OFF_gc;			// Stop LED PWM Timer;
	TCD2_CTRLB = 0x00;
	PORTD_OUT = 0x00;						// Turn off all the LEDs
}

//----------------------------------------------------------------------
//
// Prototype:	SIGNAL (RTC_OVF_vect)
//
// Description:	RTC overflow ISR
//
// Parameters:	None 
//
// Returns:	None
// 
// Notes: Interrupt occurs every 200ms
//		  Handles LED blinking at various phases of flash programming
//----------------------------------------------------------------------
SIGNAL(RTC_OVF_vect)
{
	// If comes to boot loader due to invalid Toaster firmware, fast blink Red LED
	uint8_t bootmode;
	bootmode = BL.ModeReg & BOOT_MODES_gm;
	
	if (FLASH_PROG_DISABLE == FP.State)
	{
		if (PORTD_OUT & ALL_LEDS_gm)		// If LED's are turned on
		{
			PORTD_OUTCLR = ALL_LEDS_gm;	// Turn them off.
		}
		else
		{									// If LED's are off
			if (ShowMySelf)
				PORTD_OUTSET = ALL_LEDS_gm;
			else
			{			
				if (BOOT_MODE1 == bootmode)
					PORTD_OUTSET = RED_LED_bm;	// If comes to boot loader due to bad or no Toaster firmware, fast blink Red LED
				else
					PORTD_OUTSET = GRN_LED_bm;	// If comes to boot loader due to user command, fast blink Green LED
			}
		}
		if (FAILED == I2C.ReadExtEEP)		// Check for insertion of a FUB on every 200ms
			I2C.ReadExtEEP = START;
	}
	TCD2_CTRLB = PORTD_OUT;		// Enable PWM outputs
}
