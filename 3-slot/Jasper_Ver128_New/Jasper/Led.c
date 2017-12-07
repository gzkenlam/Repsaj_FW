//-----------------------------------------------------------------------------
// FILENAME:  led.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares LED blinking routines
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "a2d.h"
#include "led.h"
#include "pwm.h"
#include "uart.h"
#include "macros.h"
#include "battcomm.h"
#include "battchrg.h"
#include "batt_defs.h"
#include "PP_BattDefs.h"
#include "jasper_ports.h"

LED_PARAM LED;	// Global data structure for LED blinking

//-----------------------------------------------------------------------------
//
// Prototype:	Init_LEDs
//
// Description:	Routine to initialize LED control variables
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:		The LEDs in first two slots (0 & 1) have inverted outputs. Therefore in order to
//				get the same intensity level they have to be subtracted by 100
// 
//-----------------------------------------------------------------------------
void Init_LEDs(void)
{
	LED_COLORS *pled;
	memset(&LED, 0, sizeof(LED));	// Clear all variables to 0
	pled = (LED_COLORS*)&LED;		// Start from Red LED pattern in slot0
	for (uint8_t slot=0; slot < MAX_SLOTS; slot++)	// Do the following for all 3 slots
	{
		pled->Red = RED_TEST_PAT;	// Set the Red LED test pattern
		pled->Grn = GRN_TEST_PAT;	// Set the Green LED test pattern
		pled++;						// Move to patterns in next slot
	}
	LED.TestPatCnt = TEST_PAT_DUR;	// Set the duration of test pattern
	
	// ---Setup Timer counter TCD2 to control LED intensities (configure as eight 8-bit PWM outputs)---
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
	
	TCD2_LCMPA = 100-DEF_INTENSITY_AMB_RED;	// Red0	- 928
	TCD2_HCMPA = DEF_INTENSITY_AMB_RED;		// Red2 - 929
	
	TCD2_LCMPB = 100-DEF_INTENSITY;			// Grn0	- 92A
	TCD2_HCMPB = DEF_INTENSITY;				// Grn2	- 92B
	
	TCD2_LCMPC = 100-DEF_INTENSITY_AMB_RED;	// Red1	- 92C
	TCD2_LCMPD = 100-DEF_INTENSITY;			// Grn1	- 92E
	
	//-------- LED Burst Timer -----------------------
	TCD1_CTRLB = TC_WGMODE_SS_gc;			// Single slope mode
	TCD1_CNT = 0;							// Clear burst timer counter
	TCD1_INTFLAGS = TC1_OVFIF_bm;			// Clear any pending burst timer overflow interrupts
	TCD1_INTCTRLA = TC_OVFINTLVL_MED_gc;	// Enable burst timer interrupt
	//------------------------------------------------
}

//----------------------------------------------------------------------
//
// Prototype:	SIGNAL (TCD1_OVF_vect)
//
// Description:	To obtain 64ms BURST Time
//
// Parameters:	None
//
// Returns:		None
//
// Notes:
//
//----------------------------------------------------------------------
SIGNAL (TCD1_OVF_vect)
{	
	TCD1_CTRLA = TC_CLKSEL_OFF_gc;	// Stop LED burst timer
	
	// Burst duration is over. Restore default LED intensities
	*LED.pBestCCReg = LED.DefIntensityRed;
	*(LED.pBestCCReg+2) = LED.DefIntensityGrn;
}

//-----------------------------------------------------------------------------
//
// Prototype:	Deinit_LEDs
//
// Description:	Routine to reset LED control variables and turn off all LED's
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:   
// 
//-----------------------------------------------------------------------------
void Deinit_LEDs(void)
{
	TCD1_CTRLA = TC_CLKSEL_OFF_gc;			// Stop Busrt duration Timer;
	TCD2_CTRLA = TC_CLKSEL_OFF_gc;			// Stop LED PWM Timer;
	
	TCD1_CTRLB = 0x00;
	TCD1_INTCTRLA = TC_OVFINTLVL_OFF_gc;	// Disable Burst Timer interrupt;
	
	TCD2_CTRLB = 0x00;
	PORTD_OUT = 0x00;						// Turn off all the LEDs
}

//-----------------------------------------------------------------------------
//
// Prototype:	DoLEDPatterns
//
// Description:	Routine to control LED's of Jasper Toaster (In All Slots)
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//
//-----------------------------------------------------------------------------
void DoLEDPatterns(void)
{
	uint8_t color = 0;
	uint8_t slot = SLOT0;
	uint8_t led_bm = _BV(0);	// Start from 0th bit position of LED Port
	uint8_t led_port;
	uint8_t *pled_pat;
	volatile uint8_t *pPortConfig;
	
	led_port = PORTD_OUT;		// Get a copy of led port
	pled_pat = &LED.Pat.Slot[SLOT0].Red;
	pPortConfig = &PORTD_PIN0CTRL;

	while (slot < MAX_SLOTS)						// Do this for LED's in all 3 slots
	{
		while (color < ALL_LED_COLORS)				// Do this for all colors (Red, Green)
		{
			uint8_t *pled_cnt;
			pled_cnt = pled_pat + ALL_LED_PATTERNS;	// Set pointer to LED counters
			if (!(*pled_pat))						// If LED pattern is OFF
				PORTD_OUTCLR = led_bm;				// Turn off the LED
			else
			{
				if (!(*pled_cnt))					// If pattern counter expires
				{
					if (!(led_port & led_bm))		// If LED was turned off
					{
						*pled_cnt = ON_TIME(*pled_pat);	// Get the LED on time
						if (*pled_cnt)				// If On time is not zero
							PORTD_OUTSET = led_bm;	// Turn on the LED
					}
					else							// If LED was on
					{
						*pled_cnt = OFF_TIME(*pled_pat); // Get the LED off time
						if (*pled_cnt)				// If off time is not zero
							PORTD_OUTCLR = led_bm;	// Turn off the LED 
					}
				}
			}
			TCD2_CTRLB = PORTD_OUT;		// Enable PWM outputs
			if (slot < SLOT2)
			{
				if (PORTD_OUT & led_bm)
					*pPortConfig = (PORT_OPC_TOTEM_gc | PORT_INVEN_bm);
				else
					*pPortConfig = PORT_OPC_TOTEM_gc;
			}
			color++;				// Move to next color
			pled_pat++;				// Increment LED Pattern pointer
			pPortConfig++;
			led_bm <<= 1;			// Shift to next bit position in LED Port
		}	
		color = 0;					// Clear color index, start from Red
		slot++;
	}
	
	if (PORTD_OUT)					// At least one LED is on
	{
		if (!TCD2_CTRLA)			// If LED PWM Timer is stopped
		{
			TCD2_HCNT = 0;			// Clear LED PWM counters
			TCD2_LCNT = 0;
			TCD2_CTRLA = TC_CLKSEL_DIV4_gc;	// Start LED PWM Timer at 30kHz	
			SetActives(LED_PWM_bm);	// Add LED PWM Timer to active peripheral list
		}
	}
	else							// If all the LEDs are off
	{
		if (TCD2_CTRLA)				// If LED PWM Timer is running
		{
			TCD2_CTRLA = TC_CLKSEL_OFF_gc;	// Stop LED PWM Timer
			uint8_t sreg = SREG;
			SREG = 0;
			SYS.Actives &= ~LED_PWM_bm;		// Remove from active peripheral list
			SREG = sreg;
		}
	}
}

void SetShowYourSelf(uint8_t state)
{
	uint8_t blink_pat = LED_OFF_PAT;
	uint8_t slots = MAX_SLOTS;
	LED_COLORS *pled;
	pled = (LED_COLORS*)&LED;
	
	LED.ShowYourSelf = state;
	if (state)
		blink_pat = MODERATE_BLINK_PAT;
	
	while (slots--)
	{
		pled->Red = blink_pat;	
		pled->Grn = blink_pat;	
		pled++;
	}
}
//-----------------------------------------------------------------------------
//
// Prototype:	SetChrgLEDPat
//
// Description:	Routine to set charger LED patterns of the current slot
//
// Parameters:	chrg_state: Charger state that matters
//
// Returns:		None
// 
// Notes: 
// 
//-----------------------------------------------------------------------------
void SetChrgLEDPat(uint8_t chrg_state)
{
	if ((LED.TestPatCnt) || (LED.ShowYourSelf))			// Return if power on reset test patterns are still on progress
		return;
		
	uint8_t sync_pat = 0;
	uint8_t red_pat = LED_OFF_PAT;	// Chrg LED patterns for charger idle
	uint8_t grn_pat = LED_OFF_PAT;
	uint8_t need_resync = FALSE;
	uint8_t curr_slot = PORTR_OUT;
	uint8_t batt_healthy = TRUE;
	uint8_t intensity = 100-DEF_INTENSITY;
	LED_COLORS *pled;

	//				Charger LED Patterns
	// |--------------|-------------------|-------------------|------------------|
	// | Chrg State	  | 	Best		  | 	Healthy		  | 	Unhealthy	 |
	// |--------------|-------------------|-------------------|------------------|
	// | Chrg Disable | All Off			  | All Off 		  | All Off 		 |
	// | In Progress  | Solid Amber+Burst |	Solid Amber	      | Solid Red	 	 |
	// | Chrg Done    | Solid Green+Burst |	Solid Green 	  | Solid Red 		 |
	// | Chrg Error   | Fast Red		  | Fast Red	      | Fast Red	 	 |
	// |--------------|-------------------|-------------------|------------------|
	
	if (BATT_TYPE_QLN == pBatt->BattType)
	{
		if (pBatt->ChrgCycleCount >= pEEP->CCThreshold)
			batt_healthy = FALSE;
	}
	else
	{
		if (pBatt->BattType > BATT_TYPE_QLN)	// ZQ3, RLX0, RLX1 and FUB
		{
			if (pBatt->SOH < pEEP->HealthThreshold)
				batt_healthy = FALSE;
		}
	}
	
	
		
	switch (chrg_state)
	{
		case CHARGING_ON_PROGRESS:			// If battery charging on progress			
			red_pat = SOLID_ON_PAT;			// Set solid on pattern to red LED
			if (batt_healthy)				// If this is a healthy battery
			{
				intensity = 100-DEF_INTENSITY_AMB_RED;
				grn_pat = SOLID_ON_PAT;		// Also set the green LED with same pattern, so that it will be solid Amber
			}
			break;

		case CHARGING_COMPLETE:				// If battery is fully charged
			if (batt_healthy)				// If this is a healthy battery
				grn_pat = SOLID_ON_PAT;		// Set solid pattern to green LED
			else
				red_pat = SOLID_ON_PAT;		// Set solid pattern to red LED if this is an unhealthy battery
			break;

		case CHARGING_ERROR:				// If there is a charging error
			red_pat = FAST_BLINK_PAT;		// Set fast blinking pattern to red LED
			sync_pat = FAST_BLINK_PAT;		// Update the sync pattern with fast blink
			break;
			
		// ---- This is to show toaster/FUB FW versions by LED blinks -----
		case BLINK_MODERATE_AMBER:			// Amber LED blinks are used to show major FW version
			intensity = 100-DEF_INTENSITY_AMB_RED;
			// No "break" here
		case BLINK_MODERATE_RED:			// Red LED blinks are used to show LSD of minor FW version
			red_pat = MODERATE_BLINK_PAT;
			// No "break" here
		case BLINK_MODERATE_GREEN:			// Green LED blinks are used to show MSD of minor FW version
			if (chrg_state != BLINK_MODERATE_RED)
				grn_pat = MODERATE_BLINK_PAT;
			break;
		// ----------------------------------------------------------------
	}

	pled = (LED_COLORS*)&LED + curr_slot;	// Start from red LED pattern in current slot
	
	if (pled->Red != red_pat)	// If there is a change in red LED pattern
	{
		pled->Red = red_pat;	// Set new red pattern
		need_resync = TRUE;		// Pattern has changed. Set resync flag
	}
	if (pled->Grn != grn_pat)	// If there is a change in green LED pattern
	{
		pled->Grn = grn_pat;	// Set new green pattern
		need_resync = TRUE;		// Pattern has changed. Set resync flag
	}

	// NOTE: Since we are using bi-color LED, two colors need to be synchronized, especially for Amber where both LEDs have to be turned on/ off at the same time
	if (need_resync)			// If new LED blinking pattern(s) to be synchronized
	{// Yes
		
		// -------- Set red LED intensity level ----------
		volatile uint8_t* pCCReg;
		uint8_t offset = curr_slot*4;;
		if (offset >= 8)
		{
			offset -= 7;
			intensity = 100 - intensity;
		}
		pCCReg = &TCD2_LCMPA + offset;
		*pCCReg = intensity;
		//-------------------------------------------------
		
		uint8_t sreg;
		uint16_t led_data;
		
		uint8_t sync_slot = MAX_SLOTS;
		uint8_t sync_led_pos = 0;
		uint8_t sync_led_cnt = 0;
		uint8_t sync_led_state = 0;
		
		if (sync_pat)			// If LED pattern sync is required
		{
			uint8_t *pled_sync_pat;
			pled_sync_pat = (uint8_t*)&LED;
			uint8_t sync_led_found = FALSE;

			for (sync_slot=SLOT0; sync_slot<MAX_SLOTS; sync_slot++)	// Scan through all all slots
			{
				for (uint8_t led=0; led<ALL_LED_COLORS; led++)		// Scan through all colors
				{
					if (sync_slot != curr_slot)			// Skip the current slot
					{
						if (*pled_sync_pat == sync_pat)	// If a LED is found with the same blinking pattern from another slot
						{
							sync_led_found = TRUE;		// Yes, set the flag and exit from the loop
							break;
						}
					}
					pled_sync_pat++;					// Move to next LED pattern
					sync_led_pos++;						// Increment the LED index
				}
				if (sync_led_found)						// Exit from the loop if sync LED is found
					break;
			}
		}

		led_data = (CHRG_LEDS_gm << (ALL_LED_COLORS * curr_slot));

		pled += MAX_SLOTS;			// Move the pointer to red LED counter in current slot
		sreg = SREG;
		SREG = 0;					// Disable global interrupts
		uint8_t led_port = PORTD_OUT;

		if (sync_slot < MAX_SLOTS)	// If sync LED is found
		{
			sync_led_cnt = *(&LED.Cnt.Slot[SLOT0].Red + sync_led_pos);	// Get the sync LED counter value
			
			sync_led_state = (led_port >> sync_led_pos) & 0x01;		// Obtain the sync LED state

			//--------------------- Sync and Inverted ------------------
			//if (ON_TIME(sync_pat) == OFF_TIME(sync_pat)) 	// If the sync pattern is symmetrical (best way to do this, but the code size is large)
			/*
			if ((ON_TIME(SLOW_BLINK_PAT) == OFF_TIME(SLOW_BLINK_PAT)) &&// If fast blinking pattern is symmetrical
			(ON_TIME(FAST_BLINK_PAT) == OFF_TIME(FAST_BLINK_PAT)))		// If slow blinking pattern is symmetrical
				sync_led_state ^= ((curr_slot%2) ^ (sync_slot%2));		// Just invert patterns in slots 1 and 3
			*/
			//----------------------------------------------------------
		}
		
		if (sync_led_state)			// If LEDs to be turned on
			PORTD_OUTSET = led_data;
		else
			PORTD_OUTCLR = led_data;
			
		pled->Red = sync_led_cnt;	// Synchronize charger LED counters
		pled->Grn = sync_led_cnt;
		SREG = sreg;				// Enable global interrupts
		DoLEDPatterns();	
	}
}
