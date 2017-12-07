//-----------------------------------------------------------------------------
// FILENAME:  timers.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares RTC ISR and hardware initialization functions
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
#include <stddef.h>
#include "a2d.h"
#include "led.h"
#include "uart.h"
#include "debug.h"
#include "macros.h"
#include "timers.h"
#include "sysparam.h"
#include "battchrg.h"
#include "battcomm.h"
#include "batt_defs.h"
#include "jasper_ports.h"
//----------------------------------------------------------------------
//
// Prototype:	SetupSystemClock
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

	// Set the PLL to 2Mhz int osc as source and mult by 12
	OSC_PLLCTRL = OSC_PLLFAC3_bm | OSC_PLLFAC2_bm;

	// Enable the PLL
	OSC_CTRL |= OSC_PLLEN_bm;

	// Check for PLL stable
	while (!(OSC_STATUS & OSC_PLLRDY_bm));

	// Switch to PLL for system clock
	CPU_CCP = CCP_IOREG_gc;
	CLK_CTRL = CLK_SCLKSEL2_bm;

	DFLLRC2M_CTRL = DFLL_ENABLE_bm;			// Enable automatic runtime calibration of 2MHz oscillator
}

//----------------------------------------------------------------------
//
// Prototype:	Init_Timers
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
	// Setup RTC to interrupt every 100ms (Blink LED's)
	OSC_CTRL |= OSC_RC32KEN_bm;				// Enable 32.768kHz Internal RC oscillator
	while (!(OSC_STATUS & OSC_RC32KRDY_bm));// Wait until 32.768kHz is stabilized

	CLK_RTCCTRL = CLK_RTCSRC_RCOSC_gc | CLK_RTCEN_bm;	// Setup RTC Clock source as 1.024kHz from 32.768kHz internal RC oscillator

	RTC_CNT		= 0; 						// Clear counter value
	while(RTC_STATUS & RTC_SYNCBUSY_bm);	// Wait till counter sync with sub clock

	RTC_PER	= CNT_100MS;					// Set period such that overflow interrupt occurs every 100ms
	RTC_COMP = CNT_100MS+1;
	while(RTC_STATUS & RTC_SYNCBUSY_bm);	// Wait till period sync with sub clock
	
	RTC_INTFLAGS= RTC_COMPIF_bm | RTC_OVFIF_bm;	// Clear any pending RTC interrupts
	RTC_INTCTRL	= RTC_OVFINTLVL_MED_gc;		// Interrupt level medium
	RTC_CTRL	= RTC_PRESCALER0_bm;		// Start RTC, Set prescale /1

	//-------- I2C Guard Timer -----------------------
	TCC1_CTRLB = TC_WGMODE_NORMAL_gc;		// Normal mode
	TCC1_PER = I2C_GUARD_TIME;				// I2C batt guard timer is set to timeout after 280ms
	TCC1_CNT = 0;
	TCC1_INTFLAGS = TC1_OVFIF_bm;
	TCC1_INTCTRLA = TC_OVFINTLVL_MED_gc;
	//------------------------------------------------
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
	// Deinit RTC timer
	RTC.CTRL = 0;
	RTC_INTCTRL	= 0;
	RTC_INTFLAGS= RTC_COMPIF_bm | RTC_OVFIF_bm;	// Clear any pending RTC interrupts

	// Deinit I2C Guard Timer
	TCC1_CTRLA = 0;
	TCC1_CTRLB = 0;
	TCC1_PER = 0;
	TCC1_CNT = 0;
	TCC1_INTCTRLA = TC_OVFINTLVL_OFF_gc;
}

//-----------------------------------------------------------------------------
//
// Prototype:	Wait_mSecs
//
// Description:	Routine to set specific delay before going to next state
//
// Parameters:	wait_time in milli seconds (1 < wait_time < 101)
//
// Returns:		None
//
// Notes: 		This routine doesn't check wait time is within the range
//-----------------------------------------------------------------------------
void Wait_mSecs(uint8_t wait_time)
{
	while(RTC_STATUS & RTC_SYNCBUSY_bm);	// Wait till period sync with sub clock
	uint8_t rtc_cnt = RTC_CNT + wait_time;	// Set wait time
	if (rtc_cnt > CNT_100MS)
		rtc_cnt -= CNT_100MS;

	RTC_COMP = rtc_cnt;
	while(RTC_STATUS & RTC_SYNCBUSY_bm);	// Wait till period sync with sub clock
	RTC_INTFLAGS |= RTC_COMPIF_bm;			// Clear pending RTC compare match interrupt
	RTC_INTCTRL	|= RTC_COMPINTLVL_LO_gc;	// Enable RTC compare match interrupt
	while(RTC_STATUS & RTC_SYNCBUSY_bm);	// Wait till period sync with sub clock	
	pBatt->I2CBusy = TRUE;					// Set I2C busy flag
}

//----------------------------------------------------------------------
//
// Prototype:	SIGNAL (RTC_COMP_vect)
//
// Description:	RTC compare match ISR to keep appropriate delay between certain I2C states
//
// Parameters:	None
//
// Returns:		None
//
// Notes:	Delays are required for EEPROM page writes/ A2D reads etc
//
//----------------------------------------------------------------------
SIGNAL(RTC_COMP_vect)
{
	RTC_INTCTRL	&= ~RTC_COMPINTLVL_LO_gc;	// Disable compare match interrupt	
	pBatt->I2CBusy = FALSE;					// Clear I2C busy flag
	if (pBatt->I2CState < BATT_FAULT)
		pBatt->I2CState++;					// Move to next I2C state
	else
	{
	#ifdef _THERM_
		// When Fuse FETs are off, the voltage at BAT+ doesn't drop immediately when battery is removed from the bay (because of very low reverse leakage current)
		// In such situations analog comparator is used to determine the presence/absence of battery thermistor.
		if (ACA_STATUS & AC_AC0STATE_bm)		// If the battery thermistor is no more
			pBatt->I2CState = BATT_REMOVED;		// Declare it as battery has been removed
		
	#endif
		SYS.NextSlot = TRUE;					// Try next slot
	}
}

//----------------------------------------------------------------------
//
// Prototype:	SIGNAL (RTC_OVF_vect)
//
// Description:	RTC overflow ISR
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes: Interrupt occurs every 100ms
//		  Used for regular house keeping activities
//		  Handle LED blinking counters A2D readings, debug messages etc
//----------------------------------------------------------------------
SIGNAL(RTC_OVF_vect)
{
	uint8_t *pled;	// Pointer to LED counters/ patterns
	uint8_t led_count, slot;

	// ------------------ Take A2D readings -------------------------------------------------	
	if (ADC_CH_MUXPOS_PIN8_gc == ADCA.CH0.MUXCTRL)	// If A2D was not taking temperature and current reading
	{
		ADCA.CTRLA = (ADC_CH2START_bm | ADC_CH1START_bm | ADC_CH0START_bm | ADC_FLUSH_bm | ADC_ENABLE_bm);
		SLEEP_CTRL = SLEEP_SEN_bm;					// Enable Sleep at idle mode
		SYS.Actives |= A2D_bm;						// Add A2D to active peripherals list
	}
	// ------------------ Handle LED Test Patterns Upon Power Up Reset ----------------------

	if (LED.TestPatCnt)								// If LED test pattern counter is not zero
	{	
		if (--LED.TestPatCnt == 0)					// Decrement the counter by 1 and see weather it is zero now
		{//Yes
			led_count = 0;
			pled = &LED.Pat.Slot[SLOT0].Red;		// Start from Red LED pattern in slot0
			while (led_count++ < ALL_LED_PATTERNS)	// Do the following for all LED patterns in all slots
			{
				*pled = LED_OFF_PAT;				// Set the pattern as LED off
				pled++;								// Move to next pattern
			}
		}
		
		// |------------|-----------|
		// | TestPatCnt | Color		|
		// |------------|-----------|
		// | 28-21		| Amber		|
		// | 21-14		| Green		|
		// | 14-7		| Red		|
		// | 7-0		| All off	|
		// |------------|-----------|
		if (LED.TestPatCnt == 21)				// If Amber color is over, change the intensities of all red LEDs to there default value
		{
			TCD2_LCMPA = 100-DEF_INTENSITY;		// Red0	- 928
			TCD2_HCMPA = DEF_INTENSITY;			// Red2 - 929
				
			TCD2_LCMPC = 100-DEF_INTENSITY;		// Red1	- 92C
			TCD2_HCMPC = DEF_INTENSITY;			// Red3 - 92D
		}	
	}

	// ----------------- Decrement LED Pattern Counters ------------------------------------
	led_count = 0;
	pled = &LED.Cnt.Slot[SLOT0].Red;		// Start from Red LED counter at Slot0
	while (led_count++ < ALL_LED_COUNTS)	// Do the following for all LED counters in all slots
	{
		register uint8_t led_cnt;
		led_cnt = *pled;			// Get the counter value

		if (led_cnt)				// If the LED counter value is above zero
			led_cnt--;				// Decrement the counter value by 1

		*pled = led_cnt;			// Save new counter value		
		pled++;						// Increment the pointer to next LED counter
	}
	// -------------------------------------------------------------------------------------

	if ((SYS.DebugCnt) && (SYS.DebugCnt != NO_DEBUG))	// If debug enabled and counter is not zero
		SYS.DebugCnt--;									// Decrement debug counter

	if (SYS.OverCurrRecoveryCnt)
		SYS.OverCurrRecoveryCnt--;
		
	if (SYS.OverCurrOneSecCnt)
		SYS.OverCurrOneSecCnt--;

	// ------------------ Do One Second Stuff ----------------------------------------------
	if (++SYS.OneSecCnt >= TEN_TICKS)				// If one second elapsed
	{// Yes
		for (slot = SLOT0; slot<MAX_SLOTS; slot++)	// Do the following for all 3 slots
		{
			if (Batt[slot].ChrgTimeSecs)			// If more time left for charging
				Batt[slot].ChrgTimeSecs--;			// Decrement charge time by one second
				
			if (Batt[slot].DeadBattChrgCnt)			// If more time left for dead battery charging
				Batt[slot].DeadBattChrgCnt--;		// Decrement dead battery charge count		
		}
		SYS.OneSecCnt = 0;							// Reset one second counter
	}
	
#ifdef _BEST_BATT_
	// ------------------- LED bursts for best battery on every 2 Seconds --------------------
	if (++SYS.BurstRepetitionCnt >= BURST_REPETITION_TIME)
	{
		if (SYS.BestSlot < MAX_SLOTS)				// If best battery is found
		{
			uint8_t burst_intensity_red = 100-BURST_INTENSITY_RED;
			uint8_t burst_intensity_grn = 100-BURST_INTENSITY_GRN;
			uint16_t burst_time;
			
			volatile uint8_t* pCCReg;
			uint8_t offset = SYS.BestSlot*4;;
			if (offset >= 8)
			{
				offset -= 7;
				burst_intensity_red = BURST_INTENSITY_RED;
				burst_intensity_grn = BURST_INTENSITY_GRN;
			}
			pCCReg = &TCD2_LCMPA + offset;
			
			// Remember default intensity levels and corresponding CC Reg
			LED.pBestCCReg = pCCReg;
			LED.DefIntensityRed = *pCCReg;
			LED.DefIntensityGrn = *(pCCReg+2);
			
			// If this is burst Amber
			if ((DEF_INTENSITY_AMB_RED == LED.DefIntensityRed) || ((100 - DEF_INTENSITY_AMB_RED) == LED.DefIntensityRed))
			{// Yes
				//NOTE: This is possible because BURST_INTENSITY_AMB_GRN equals to 50%. Not valid for other values
				burst_intensity_grn = BURST_INTENSITY_AMB_GRN;
				burst_time = BURST_TIME_AMBER;
			}
			else
				burst_time = BURST_TIME_GREEN;
			
			TCD1_PER = burst_time;					// Set LED burst duration
			
			*pCCReg = burst_intensity_red;			// Set red LED burst intensity
			*(pCCReg+2) = burst_intensity_grn;		// Set green LED burst intensity
			
			TCD1_CTRLA = TC_CLKSEL_DIV1024_gc;		// Start LED burst timer
		}
		SYS.BurstRepetitionCnt = 0;					// Reset burst repetition count
	}
	// -------------------------------------------------------------------------------------
#endif
}
