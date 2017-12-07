//-----------------------------------------------------------------------------
// FILENAME:  a2d.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares A2D initialization and ISR functions
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
#include <avr/pgmspace.h>
#include <string.h>
#include <stddef.h>
#include "a2d.h"
#include "led.h"
#include "pwm.h"
#include "uart.h"
#include "debug.h"
#include "macros.h"
#include "eeprom.h"
#include "thermdefs.h"
#include "sysparam.h"
#include "battcomm.h"
#include "battchrg.h"
#include "battfound.h"
#include "batt_defs.h"
#include "PP_BattDefs.h"
#include "jasper_ports.h"

A2D_PARAM A2D;	// Global structure to hold A2D variables

//----------------------------------------------------------------------
//
// Prototype:	Init_A2D
//
// Description:	Routine to initialize A2D Converter
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Init_A2D(void)
{
	memset(&A2D, 0, sizeof(A2D_PARAM));	// Clear A2D global data structure

	NVM_CTRLB = NVM_EEMAPEN_bm;			// Memory mapped EEPROM
	for (uint8_t slot=SLOT0; slot<MAX_SLOTS; slot++)
	{
		A2D.Slot[slot].VSenseConst = ReadEEPWord(slot);	// Load constants for voltage sense from the EEPROM of the micro
		A2D.Slot[slot].ISenseConst = ReadEEPWord(slot+INDEX_I_SENSE_CONST);// Load constants for current sense from the EEPROM of the micro
	}
		
	// Load A2D calibration data from production signature row
	ADCA.CALL = ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, ADCACAL0));
	ADCA.CALH = ReadCalibrationByte(offsetof(NVM_PROD_SIGNATURES_t, ADCACAL1));

	ADCA.CTRLA 		= 0;
	ADCA.CTRLB 		= ADC_CONMODE_bm | ADC_RESOLUTION_12BIT_gc;	// 12bit resolution, signed mode
	ADCA.REFCTRL 	= ADC_REFSEL_INTVCC_gc;						// Set A2D reference = AVCC/1.6
	ADCA.EVCTRL 	= ADC_EVSEL_0123_gc | ADC_EVACT_NONE_gc;
	ADCA.PRESCALER 	= ADC_PRESCALER_DIV64_gc;					// Set A2D clock fclk/64 = 187.5kHz
	ADCA.INTFLAGS 	= ADC_CH2IF_bm | ADC_CH1IF_bm | ADC_CH0IF_bm;
	
	// Configure ADC channel 0 to measure battery voltage at slot0
	ADCA.CH0.CTRL 	 = ADC_CH_INPUTMODE_SINGLEENDED_gc;			// Single ended
	ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN8_gc;					// Positive input = PB0
	
	// Configure ADC channel 1 to measure battery voltage at slot1
	ADCA.CH1.CTRL 	 = ADC_CH_INPUTMODE_SINGLEENDED_gc;			// Single ended
	ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN9_gc;					// Positive input = PB1
	
	// Configure ADC channel 2 to measure battery voltage at slot2
	ADCA.CH2.CTRL 	 = ADC_CH_INPUTMODE_SINGLEENDED_gc;			// Single ended
	ADCA.CH2.MUXCTRL = ADC_CH_MUXPOS_PIN11_gc;					// Positive input = PB3
	ADCA.CH2.INTCTRL = ADC_CH_INTMODE_COMPLETE_gc | ADC_CH_INTLVL_LO_gc;	// ADC complete interrupt and low level
	
	#ifdef _THERM_
	ACA_AC0MUXCTRL = AC_MUXPOS_PIN1_gc | AC_MUXNEG_SCALER_gc;
	ACA_CTRLA = 0;
	ACA_CTRLB = 0x23;
	ACA_AC0CTRL = AC_HYSMODE_SMALL_gc | AC_ENABLE_bm;
	#endif
}

//----------------------------------------------------------------------
//
// Prototype:	Deinit_A2D
//
// Description:	Routine to de-initialize A2D converter
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Deinit_A2D(void)
{
	ADCA.CTRLA 	= 0;
	ADCA.CTRLB 	= 0;
	ADCA.REFCTRL= 0;
	ADCA.EVCTRL = 0;
	ADCA.PRESCALER= 0;
	
	ADCA.CH0.CTRL = 0;
	ADCA.CH1.CTRL = 0;
	ADCA.CH2.CTRL = 0;

	ADCA.CH0.INTCTRL = 0;
	ADCA.CH1.INTCTRL = 0;
	ADCA.CH2.INTCTRL = 0;

	// Clear pending A2D interrupts (in all 4 channels)
	ADCA.INTFLAGS = ADC_CH0IF_bm | ADC_CH1IF_bm | ADC_CH2IF_bm;
}

//----------------------------------------------------------------------
//
// Prototype:	ReadCalibrationByte
//
// Description:	Routine to get A2D calibration byte
//
// Parameters:	index: High/Low address of the A2D calibration byte
//
// Returns:		Calibration byte
// 
// Notes:
//----------------------------------------------------------------------
uint8_t ReadCalibrationByte(uint8_t index)
{
	uint8_t result;
	while (NVM_STATUS & NVM_NVMBUSY_bm);	// Wait if NVM is busy
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;	// Set read calibration row command
	result = pgm_read_byte(index);			// Read calibration byte
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;
	return result;
}

//----------------------------------------------------------------------
//
// Prototype:	SetA2DMuxForCurrTmp
//
// Description:	Routine to set A2D channels 0 and 1 to measure battery current and temperature
//
// Parameters:	None
//
// Returns:		TRUE: if successfully configure A2D channels
//				FALSE: Unable to configure A2D channels at this moment
//
// Notes:
//----------------------------------------------------------------------
uint8_t SetA2DMuxForCurrTmp(void)
{
	uint8_t ret = FALSE;
	uint8_t sreg = SREG;
	SREG = 0;
	if (!ADCA.CTRLA)		// If the A2D was not busy
	{// Yes
		A2D.BattISense = 0;
		A2D.BattTSense = 0;
		ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN0_gc;	// Configure ADC channel 0 to measure battery current
		ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN1_gc;	// Configure ADC channel 1 to measure thermistor resistance
		ret = TRUE;
	}
	SREG = sreg;
	return (ret);
}

//----------------------------------------------------------------------
//
// Prototype:	ReadA2DCurrTmp
//
// Description:	Routine to start A2D converter to take current and temperature from respective A2D channels
//
// Parameters:	next_i2c_state: Next I2C state after all A2D reading are taken
//
// Returns:		TRUE: if successfully start A2D converter
//				FALSE: Unable to start A2D converter as channels are were not configured correctly
//
// Notes:
//----------------------------------------------------------------------
uint8_t ReadA2DCurrTmp(uint8_t next_i2c_state)
{
	if (ADC_CH_MUXPOS_PIN0_gc == ADCA.CH0.MUXCTRL)	// If this a measurement of Temperature and current from the A2D
	{
		pBatt->I2CState = next_i2c_state;
		pBatt->I2CBusy = TRUE;						// Set battery I2C busy flag
		ADCA.INTFLAGS 	= ADC_CH3IF_bm | ADC_CH2IF_bm | ADC_CH1IF_bm | ADC_CH0IF_bm;
		ADCA.CH1.INTCTRL = ADC_CH_INTMODE_COMPLETE_gc | ADC_CH_INTLVL_LO_gc;	// Enable ADC complete interrupt and low level for channel 1
		
		ADCA.CTRLA = (ADC_CH1START_bm | ADC_CH0START_bm | ADC_FLUSH_bm | ADC_ENABLE_bm);
		SetActives(A2D_bm);							// Add ADC to active peripherals list
		return TRUE;
	}
	return FALSE;
}
//----------------------------------------------------------------------
//
// Prototype: 	SIGNAL (ADCA_CH1_vect)
//
// Description:	A2D channel1 conversion complete ISR
//
// Parameters:	None
//
// Returns:		None
//
// Notes:
//----------------------------------------------------------------------
SIGNAL (ADCA_CH1_vect)
{
	if (A2D.AvgCnt)	// Discard the very first (0th) reading
	{
		A2D.Slot[CUR_BUF].Sum += GetA2DVal(CUR_BUF);	// Get the summation of 8 A2D readings of charging current
		A2D.Slot[TMP_BUF].Sum += GetA2DVal(TMP_BUF);	// Get the summation of 8 A2D readings of Thermistor current
	}
	
	if (MAX_SUM_COUNT == A2D.AvgCnt)				// If 8 readings were taken
	{
		A2D.AvgCnt = 0;								// Clear average counter
		ADCA.CTRLA = 0;
		ADCA.CH1.INTCTRL = ADC_CH_INTLVL_OFF_gc;	// Disable A2D channel 1 complete interrupt
		
		ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN8_gc;	// Configure ADC channel 0 to measure battery voltage at slot 0
		ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN9_gc;	// Configure ADC channel 1 to measure battery voltage at slot 1

		A2D.BattISense = A2D.Slot[CUR_BUF].Sum / MAX_SUM_COUNT;	// Get the average value of current sense
		A2D.BattTSense = A2D.Slot[TMP_BUF].Sum / MAX_SUM_COUNT;	// Get the average value of temperature sense

		if (A2D.BattTSense >= K)				// We should not see this
			A2D.BattTSense = K-1;

		if (A2D.BattISense < 0)					// Since the A2D runs on signed mode, this can go slightly bellow zero when no current is flowing
			A2D.BattISense = 0;					// If so, make it 0.

		A2D.Slot[CUR_BUF].Sum = 0;				// Clear summation
		A2D.Slot[TMP_BUF].Sum = 0;

		pBatt->I2CBusy = FALSE;					// Clear I2C busy flag
		SYS.Actives &= ~A2D_bm;					// Clear A2D from active peripherals list

		SYS.SkipSleep = TRUE;
	}
	else
	{
		ADCA.CTRLA = (ADC_CH1START_bm | ADC_CH0START_bm | ADC_ENABLE_bm); // Take next A2D readings
		A2D.AvgCnt++;				// Increment average counter
	}
}

//----------------------------------------------------------------------
//
// Prototype: 	SIGNAL (ADCA_CH2_vect)	
//
// Description:	A2D channel2 conversion complete ISR
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
SIGNAL (ADCA_CH2_vect)
{
	if (A2D.AvgCnt)	// Discard the very first (0th) reading
	{
		for (uint8_t slot=SLOT0; slot<MAX_SLOTS; slot++)
			A2D.Slot[slot].Sum += GetA2DVal(slot);	// Get the summation of 8 A2D readings for all 3 slots
	}
	
	if (MAX_SUM_COUNT == A2D.AvgCnt)				// If 8 readings were taken
	{
		uint8_t fast_chrg_batt_cnt = 0;				// Clear fast charging battery counter
		A2D.AvgCnt = 0;								// Clear average counter
		ADCA.CTRLA = 0;								// Disable A2D converter
		
		// The total current drawn from all 3 slots not to exceed 4500mA (1500 mA per slot)
		SYS.MaxFastChrgCurr = SUM_OF_FAST_CHRG_CURR;	// Sum of charging currents of all 3 slots
		
		BATT_DATA *pbatt;
		pbatt = Batt;
		
		A2D_P *pa2d;
		pa2d = (A2D_P*)&A2D;
		
		for (uint8_t slot=SLOT0; slot<MAX_SLOTS; slot++)	// Do the following for all 3 slots
		{
			int16_t batt_v_sense;
			pa2d->Sum /= MAX_SUM_COUNT;				// Get the average value
			batt_v_sense = (pa2d->VSenseConst * (int32_t)pa2d->Sum)/A2D_RES;	// Calculate the voltage at BAT+

			pa2d->Sum = 0;							// Clear summation

			// Calculate voltage ramp
			int16_t v_sense_ramp = batt_v_sense - pa2d->BattVSense;	// delta v sense per 100ms

			if (v_sense_ramp < 0)					// if the ramp is negative
				v_sense_ramp = -v_sense_ramp;		// Make it positive
			
			uint8_t batt_flags = (BATT_FLAGS << slot);

			if (!(SYS.BattDetect & batt_flags))		// If none of the battery flags were set (which means there is no battery in the bay)
			{// Yes
				if ((batt_v_sense > BAT_DETECT_LO) && (batt_v_sense < BAT_DETECT_HI) && (v_sense_ramp < _100mV)) // If (0.1V < BAT+ < 9.7V) and BAT+ is stable enough
				{
					
					if (!pbatt->WaitCnt)			// If battery detect time elapsed? (First step to address the chattering issue)
					{
						#ifdef _DEBUG_MSGS_
						SendRS232('d');
						#endif
						SYS.BattDetect |= batt_flags & 0x0F;	// Set battery insert flag
					}
				}
				else
					pbatt->WaitCnt = BATT_DETECT_TIME;	// Reset battery detect count
			}
			else
			{
				if ((pbatt->I2CState >= BATT_FAULT) && (pbatt->I2CState <= SIMPLY_WAIT))	// A battery with one of the fault state has been set
				{
					if (!(PORTA_OUT & (CHG_DIS0_bm << slot)))			// Case 1: if fuse FET is on
					{
						if ((v_sense_ramp > FUSE_ON_RAMP) || (batt_v_sense > BAT_REMOVE_HI))	// If the BAT+ rising ramp > 500mV or BAT+ rises above 10.7V
							pbatt->I2CState = BATT_REMOVED;	// Battery has been removed
					}
					#ifndef _THERM_
					else								// Case 2: if fuse FET was on
					{
						int16_t fuse_off_ramp = FUSE_OFF_RAMP;
						
						if (batt_v_sense >= 2500)
							fuse_off_ramp += 70*((batt_v_sense/500) - 4);	// Add additional 70mV for each 500mV step in batt v sense

						// Note: Fuse off ramp (always falling) is critical due to following reason
						// If charging error occurs while battery is fast charging at higher current (say 1.5 Amps), the battery
						// voltage drops quickly as fuse FET goes off.This drop should not be misjudged as a battery removal especially
						// at low temperatures where battery impedance is high.

						// The values in following table are obtained experimentally.
						// |-------------------|-------------------|
						// | batt v sense(mV)  | fuse off ramp(mV) |
						// |-------------------|-------------------|
						// | 2000 <= 	< 2500 |		175		   |
						// | 2500 <=	< 3000 |		245		   |
						// | 3000 <=	< 3500 |		315		   |
						// | 3500 <=	< 4000 |		385		   |
						// | 4000 <=	< 4500 |		455		   |
						// | 4500 <=	< 5000 |		525		   |
						// |-------------------|-------------------|
						
						if ((v_sense_ramp  > fuse_off_ramp) || (batt_v_sense < BAT_REMOVE_LO))	// If the BAT+ falling ramp > 200mV or BAT+ fall bellow 2.0V
							pbatt->I2CState = BATT_REMOVED;		// Battery has been removed
					}
					#endif
					
					if (COMM_FAULT == pbatt->I2CState)		// If charger encounter a communication fault
					{
						if (!pbatt->WaitCnt)				// If battery was not removed within the given time
						{
							if (++pbatt->CommRetryCnt == MAX_COMM_RETRY_CNT)
							{
								// Must be something wrong with the battery chips
								pbatt->Fault = BATT_COMM_FAULT;
								pbatt->I2CState = BATT_FAULT;	// Conclude this as truly a communication failure
							}
							else
							{
								// Start things from the beginning without reseting the "CommRetryCnt" (Second step to address the chattring issue)
								pbatt->I2CState = BATT_REMOVED_VIRTUAL;
							}
						}
					}
				}
			}
			
			// Adjustments to battvsense if the voltage pwm is on
			if ((pbatt->ChrgState >= START_SLOW_CHRG) && (pbatt->ChrgState <= CHRG_NEARLY_DONE))	// If battery is charging
			{
				if (pbatt->ChrgState < START_FAST_CHRG)					// If one or more batteries are slow charging
					SYS.MaxFastChrgCurr -= EEP[slot].SlowChrgCurr;		// Reduce slow charge current from the total
				else
				{
					if (pbatt->ChrgState <  CHRG_NEARLY_DONE)			// If some batteries are fast charging
						fast_chrg_batt_cnt++;							// Increment the fast charge battery count
					else
						SYS.MaxFastChrgCurr -= EEP[slot].NearlyDoneCurr;//	Reduce the nearly done current from the total if there are batteries that are almost fully charged,
				}
				
				if (EEP[slot].ChrgupVolt > PWM_V_OFF_VOLT)	// If voltage pwm is on
					batt_v_sense = ((int32_t)batt_v_sense * EEP[slot].ChrgupVolt)/PWM_V_OFF_VOLT;	// Account for attenuation
			}
			
			pa2d->BattVSense = batt_v_sense;	// Update the global storage
			if (pbatt->WaitCnt)
				pbatt->WaitCnt--;
			pbatt++;
			pa2d++;
		}
		
		if (fast_chrg_batt_cnt)		// If one or more batteries are fast charging
			SYS.MaxFastChrgCurr /= fast_chrg_batt_cnt;	// Dynamically adjust maximum fast charge current

		SYS.ScanSlots = TRUE;	// It's time to scan battery slots
		SYS.SkipSleep = TRUE;	// Just in case skip the sleep
		SYS.Actives &= ~A2D_bm;	// Clear A2D from active peripherals list
	}
	else
	{
		ADCA.CTRLA = (ADC_CH2START_bm | ADC_CH1START_bm | ADC_CH0START_bm | ADC_ENABLE_bm); // Take next A2D readings
		A2D.AvgCnt++;			// Increment average counter
	}
}
