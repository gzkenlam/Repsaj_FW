//-----------------------------------------------------------------------------
// FILENAME:  battchrg.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares battery charging functionality
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
#include "led.h"
#include "a2d.h"
#include "pwm.h"
#include "uart.h"
#include "debug.h"
#include "eeprom.h"
#include "macros.h"
#include "Timers.h"
#include "thermdefs.h"
#include "sysparam.h"
#include "battchrg.h"
#include "battcomm.h"
#include "battfound.h"
#include "batt_defs.h"
#include "PP_BattDefs.h"
#include "jasper_ports.h"
#include "I2C_Batt.h"

//----------------------------------------------------------------------
//
// Prototype:	Init_BattCharger
//
// Description:	Routine to initialize battery charger variables, data structure and pointers
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void Init_BattCharger(void)
{
	// Clear global data storage
	memset(&SYS, 0, sizeof(SYSTEM_PARAM));
	memset(EEP , 0, sizeof(EEP));
	memset(Batt, 0, sizeof(Batt));
	memset(&Gauge, 0, sizeof(Gauge));
	pBatt = Batt + MAX_SLOTS;			// Set global pointer to Batt structure
	pEEP = EEP;							// Set global pointer to EEP structure
	for (uint8_t slot=SLOT0; slot<MAX_SLOTS; slot++)
	{
		pBatt--;
		pBatt->Tmp = INVALID_TMP;		// Make battery temperature invalid in all 3 slots
		pBatt->WaitCnt = BATT_DETECT_TIME;
	}
	SYS.BestSlot = MAX_SLOTS;
}

//----------------------------------------------------------------------
//
// Prototype:	GetBestBattSlot
//
// Description:	Routine to determine best battery slot
//
// Parameters:	None
//
// Returns:		best battery slot
//				0-3: If valid best battery slot is found
//				4: If there is no best battery
// Notes:
//----------------------------------------------------------------------
#ifdef _BEST_BATT_
void FindBestBattSlot(void)
{
	BATT_DATA *pbatt;
	uint8_t best_soh = 0;
	uint8_t best_chrgstate = 0;
	uint8_t best_slot = MAX_SLOTS;
	uint32_t best_figure_of_merit = 0;
	uint16_t highest_design_cap = 0;
	pbatt = Batt;
	
	// First find the highest design cap
	for (uint8_t slot=SLOT0; slot<MAX_SLOTS; slot++)
	{
		if (highest_design_cap < pbatt->DesignCap)
			highest_design_cap = pbatt->DesignCap;
		pbatt++;
	}
	pbatt = Batt;
	
	for (uint8_t slot=SLOT0; slot<MAX_SLOTS; slot++)
	{
		// In order to qualify for best of 4 indication, a battery must fulfill following requirements
		// 1) Battery gas gauge must be ready (i.e. SOH reported by gauge is correct)
		// 2) Battery should not under monthly calibration process
		// 3) Battery is not over/ under temperature condition
		// 3) Battery should be healthy
		if ((!pbatt->WaitCnt) && (pbatt->ChrgState != BAD_TMP))
		{
			uint32_t figure_of_merit = (uint32_t)pbatt->SOC * pbatt->SOH * pbatt->DesignCap;
			//uint32_t fig_of_merit_threshold = (uint32_t)pbatt->FOMThreshold * highest_design_cap * 100;
			uint32_t fig_of_merit_threshold = (uint32_t)highest_design_cap * 8000;
			
			if (figure_of_merit >= fig_of_merit_threshold)
			{
				if (best_figure_of_merit == figure_of_merit)	// In case figure of merits are equal
				{
					if (pbatt->SOH > best_soh)					// If this battery is healthier than previously selected best battery
					{
						best_slot = slot;						// Yes, selected the battery that has highest SOH
						best_soh = pbatt->SOH;
					}
					else
					{
						if (pbatt->SOH == best_soh)				// If state of healths are also equal
						{
							if (pbatt->ChrgState >  best_chrgstate)	// Check the charger state
								best_slot = slot;	
						}
					}
				}
				
				if (best_figure_of_merit < figure_of_merit)
				{
					if (pbatt->SOH >= 80)	// If this is a healthy battery
					{// Yes
						best_figure_of_merit = figure_of_merit;
						best_slot = slot;
						best_soh = pbatt->SOH;
						best_chrgstate = pbatt->ChrgState;
					}
				}
			}
		}
		pbatt++;
	}
	SYS.BestSlot = best_slot;
}
#endif

//----------------------------------------------------------------------
//
// Prototype:	ScanChrgSlots
//
// Description:	Routine to scan charger slots
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
uint8_t u8Retry = 5;

void ScanChrgSlots(void)
{
	uint8_t curr_slot = PORTR_OUT;
	uint8_t batt_flags = (BATT_FLAGS << curr_slot);
	uint8_t comm_retry_cnt = 0;
	uint8_t comm_err_i2c_state = 0;
	//uint8_t temp_i2c_state = 0;
	
	if (!PORTB_INTCTRL)					// If over current comparator was tripped
	{
		PORTA_OUTSET = CHG_DIS_gm;		// Disable all chargers/ battery detect
		PORTA_OUTCLR = CMOD_ALL_gm;		// Make all CMOD pins low
		pBatt->I2CState = BATT_FAULT;	// Go to battery fault state
		pBatt->Fault = OVER_CURR_FAULT;	// Report the fault state

		// The following logic is in place in order to recover the system from ESD test OR lock the system in case of a true short circuit condition
		if ((!SYS.OverCurrRecoveryCnt) && (SYS.OverCurrRepeatCnt))	// If 5 seconds elapsed and repeat count is not zero
		{
			BATT_DATA *pbatt;
			pbatt = Batt;
			uint8_t slots = MAX_SLOTS;

			while (slots--)
			{
				pbatt->I2CState = BATT_REMOVED;	// Set all slots with battery remove state so that charging will restart from the begining
				pbatt++;
			}

			SYS.OverCurrOneSecCnt = CNT_ONE_SEC;// Start one second timer

			CLEAR_PENDING_OVER_CURR_INTS;		// Clear pending over current interrupts
			ENABLE_OVER_CURR_INT;				// Re enable over current interrupt
			u8Retry = 7;						// error retry will be reloaded 
		}
	}
	else
	{
		if ((LED.TestPatCnt) || (!SYS.ScanSlots) || (pBatt->I2CBusy))	// If this is not the time to scan slots or I2C communication is on progress
			return;									// Just return

		if (pBatt->I2CState < BATT_REMOVED)			// As long as no faults (battery/charger)
		{
			uint8_t batt_detect = SYS.BattDetect;
			
			if (batt_detect & (batt_flags & 0x0F))	// If battery insert flag is set
			{
				pBatt->Fault = CheckBattFound();	// Call battery found
				if (pBatt->Fault){					// If battery fault was returned
					pBatt->I2CState = BATT_FAULT;	// Set next I2C state
					//adding code for retry after error is found during ESD recover
					
				}
				else{
					//u8Retry = 5;		move the retry recover to other place
				}
					
			}
			else
			{
				if (batt_detect & (batt_flags & 0xF0))	// If battery present flag is set
					Do_I2C();						// Do I2C calls to retrieve battery voltage, current etc.
				else
				{
					SYS.NextSlot = TRUE;			// No battery in this slot, try next slot
					return;							// Return
				}
			}
		}
	}

	// --------------- Handle battery / charger faults here ------------------
	switch (pBatt->I2CState)
	{
		// For all types of battery or charger faults
		case BATT_FAULT:
		case CHRG_FAULT:
		if((pBatt->Fault == SLOW_TIME_FAULT)||(pBatt->Fault == FAST_TIME_FAULT))
			{
				PORTA_OUTSET = (CHG_DIS0_bm << curr_slot);	// Turn off the charger
				pBatt->SOC = 0;								// Clear state of charge
				SetChrgLEDPat(CHARGING_ERROR);				// Start blinking error pattern
			}
			else 
			{
				if(u8Retry){
					u8Retry--;
					Deinit_I2CBatt();	// Reinitialize I2C battery interface
					Init_I2CBatt();
					//Update_I2CBattStatus(NACK_bm);	// Update the status with NACK
					//SendRS232('Z');
					//SendRS232(u8Retry + 0x30);
					pBatt->I2CState = BATT_REMOVED;
				}
				else{
					//u8Retry = 5;
					PORTA_OUTSET = (CHG_DIS0_bm << curr_slot);	// Turn off the charger
					pBatt->SOC = 0;								// Clear state of charge
					SetChrgLEDPat(CHARGING_ERROR);				// Start blinking error pattern
					//SendRS232('R');
				}
		   }	
			// No "break" here
			
		case COMM_FAULT:
			SetPWMI(curr_slot, 0);						// Turn off current PWM
			SetPWMV(curr_slot, 0);						// Turn off voltage PWM
			ADCA.CH1.MUXCTRL = ADC_CH_MUXPOS_PIN9_gc;	// Configure ADC channel 1 to measure battery voltage at slot 1
			ADCA.CH0.MUXCTRL = ADC_CH_MUXPOS_PIN8_gc;	// Configure ADC channel 0 to measure battery voltage at slot 0
			Wait_mSecs(CNT_10MS);						// Wait 10ms before check thermistor state
			break;
		
		case BATT_REMOVED_VIRTUAL:
			comm_retry_cnt = pBatt->CommRetryCnt;		// Backup comm fault retry count
			comm_err_i2c_state = pBatt->CommErrI2CState;
			// No "break" here

		case BATT_REMOVED:
			PORTA_OUTCLR = ((CHG_DIS0_bm | CMOD0_bm ) << curr_slot);// Turn on fuse FET to detect next battery insertion and also switch to pre-charge current mode
			//if (BATT_REMOVED == pBatt->I2CState)		// If this is identified as true battery removal
				SetChrgLEDPat(CHARGING_DISABLE);		// Turn off charger LEDs
			//SendHex(pBatt->I2CState);
			memset(pBatt, 0, sizeof(BATT_DATA));		// Clear global battery data storage structure corresponding to current slot
			pBatt->Tmp = INVALID_TMP;					// Make the temperature invalid
			pBatt->WaitCnt = BATT_DETECT_TIME;
			pBatt->CommRetryCnt = comm_retry_cnt;
			pBatt->CommErrI2CState = comm_err_i2c_state;
			memset(pEEP, 0, sizeof(EEP_CHRG_DATA));		// Clear global EEPROM data storage structure corresponding to current slot
			{
				DEBUG_DATA *pDD;
				pDD = DD+curr_slot;
				memset(pDD, 0, sizeof(DEBUG_DATA));
			}
			
			uint8_t sreg = SREG;
			SREG = 0;								// Disable global interrupts
			SYS.BattDetect &= ~batt_flags;			// Clear battery flags
			SREG = sreg;							// Enable global interrupts
#ifdef _DEBUG_MSGS_
			SendRS232('r');
#endif
			// No "break" here

		case SIMPLY_WAIT:							// End of toaster firmware update
			ShowFWVersion();						// Simply do nothing but show Toaster/FUB firmware version
			SYS.NextSlot = TRUE;					// Try next slot
			break;
	}
}

//----------------------------------------------------------------------
//
// Prototype:	DoBattChrg
//
// Description:	Routine to call battery charger and updates the fault state
//
// Parameters:	next_i2c_state: Next I2C state to be set after calling battery charger 
//
// Returns:		None
// 
// Notes: 	Following parameters of the "Gauge" data structure must be filled with appropriate values before calling this function
//			(FlagsLo, FlagsHi, SOC, Volt and AvgCurr)
//			Also pBatt->Tmp and pBatt->SOH must be set with proper values bafore calling this function
//----------------------------------------------------------------------
void DoBattChrg(uint8_t next_i2c_state)
{
	pBatt->Fault = DoCharger();					// Call battery charger
	if (!pBatt->Fault)							// If there is no charger fault
		pBatt->I2CState = next_i2c_state;		// Set next i2c state	
	else										// If charging error
		pBatt->I2CState = CHRG_FAULT;			// Goto charger fault state

	// Refer the function "PrintBattChrgInfo" in "debug.c" just to understand the following black magic
	SYS.NextSlot = TRUE + TRUE;			// First True is to switch the slot, second True tells that Gauge structure has good data corresponding to this slot
}

//----------------------------------------------------------------------
//
// Prototype:	DoCharger
//
// Description:	Battery charging state machine
//
// Parameters:	None
//
// Returns:		Charger error
// 
// Notes:
//----------------------------------------------------------------------
uint8_t DoCharger(void)
{
	uint8_t curr_slot = PORTR_OUT;
	// ---------------------------------------------------------------------------------------------------------
	if (Gauge.AvgCurr < 0)
		Gauge.AvgCurr = 0;
		
	if (Gauge.GGInstCurr < 0)
		Gauge.GGInstCurr = 0;
		
	if (Gauge.GGInstCurr > pEEP->AbnormalChrgCurr)		// If battery charging current exceeds abnormal current limit
		return ABNORMAL_CURR_FAULT;					// return abnormal current fault

	// If battery temperature is above the hot off limit or bellow the cold off limit
	if ((pBatt->Tmp >= pEEP->HotOff) || (pBatt->Tmp <= pEEP->ColdOff))
	{
		//SendDig(pBatt->Tmp, 3);
		ShutdownCharger();							// Shutdown the battery charger
		pBatt->ChrgState = BAD_TMP;					// Set next charger state
	}

	switch (pBatt->ChrgState)
	{	
		case REQUEST_TO_CHRG:
			PORTA_OUTCLR = (CHG_DIS0_bm << curr_slot);	// Turn on fuse FETs (In case if we come here from BAD_TMP/ CHRG_PAUSE states)
			PORTA_OUTCLR = (CMOD0_bm << curr_slot);		// Start with pre-charge mode
			pBatt->Volt_PWM_Off = 0;					// Clear PWM off voltage before testing the voltage PWM circuit
			pBatt->ChrgState = TEST_PWM_V_CIRCUIT;		// Set next charger state
			break;
			
		case TEST_PWM_V_CIRCUIT:
			{
				A2D_P *pa2d;
				pa2d = (A2D_P*)&A2D + curr_slot;
				
				int16_t batt_v_sense = pa2d->BattVSense;	// Get the battery voltage sensed by A2D

				if (GetPWMV(curr_slot))  	// If voltage PWM is turned on
				{// Yes
					// ATTN = (2000+499)/(2000+499+4750) = 2499/7249
					// 0.9*ATTN = 2249/7249
					// 1.1*ATTN = 2499/6590
					if (!pBatt->WaitCnt)
					{
						SetPWMV(curr_slot, 0);	// Turn off voltage PWM

						int16_t pwm_compare_low, pwm_compare_high;
						pwm_compare_low  = ((int32_t)pBatt->Volt_PWM_Off * 2249)/7249;
						pwm_compare_high = ((int32_t)pBatt->Volt_PWM_Off * 2499)/6590;
					
						// If the voltage attenuation is not within the low and high limits
						if ((batt_v_sense <= pwm_compare_low) || (batt_v_sense >= pwm_compare_high))
						{
							pa2d->BattVSense = pBatt->Volt_PWM_Off; // This prevents the software from seeing a fake battery removal
							if (pBatt->BattType != BATT_TYPE_DEAD)	// If this not a dead battery
								return PWM_V_FAULT;					// Return voltage PWM fault state
						}
				
						// Our charger hardware is good, continue with slow charging
						SetPWMI(curr_slot, _750mA);				// Set PWM I value (Something above 600mA as ISET1 needs to be above 120mV to turn on the charger)
						pBatt->ChrgState = SET_SLOW_CHRG_PWM;	// Set next charger state
						SetChrgLEDPat(CHARGING_ON_PROGRESS);	// Set LED blinking pattern
					}
				}
				else
				{
					int16_t delta_batt_v_sense;
					delta_batt_v_sense = batt_v_sense - pBatt->Volt_PWM_Off;
					if (delta_batt_v_sense < 0)				// If the difference is negative
						delta_batt_v_sense = -delta_batt_v_sense;	// Make it positive

					if (delta_batt_v_sense < _100mV)		// If the battery voltage is settled
					{
						SetPWMV(curr_slot, MAX_PWM_V);		// Turn on the maximum voltage PWM
						pBatt->WaitCnt = 2;
					}
				
					pBatt->Volt_PWM_Off = batt_v_sense;		// Remember the last PWM off voltage
					
				}
			}
			break;
		
		case SET_SLOW_CHRG_PWM:
			{
				// ----- Set the charge-up voltage ----
				SetChrgupVoltage();
				// ----- Set the slow charge current ----
				if (pEEP->SlowChrgCurr >= 275)
					SetPWMI(curr_slot, pEEP->SlowChrgCurr);	// Set PWM I value corresponding to slow charge current
				pBatt->AvgCurrSettleCnt = SLOW_SETTLE_TIME; // Give some time to settle the slow charge current
				pBatt->ChrgState = START_SLOW_CHRG;			// Set next charger state
			}
			break;
		
		case START_SLOW_CHRG:
			if (pEEP->SlowChrgCurr >= 275)
				PORTA_OUTSET = (CMOD0_bm << curr_slot);		// CMOD High
			pBatt->ChrgTimeSecs = pEEP->SlowChrgMins * 60;	// Set slow charge timeout
			pBatt->PWMSettleCnt = 0;
			pBatt->NearlyChrgedCnt = 0;
			pBatt->ChrgState = DO_SLOW_CHRG;				// Set next state
			break;

		case DO_SLOW_CHRG:
			// -------- Check for slow charge timeout --------
			if (!pBatt->ChrgTimeSecs)						// If slow charge time elapsed
				return SLOW_TIME_FAULT;						// Yep, return slow time fault

			// -------- Check for slow current fault ---------
			if (!pBatt->AvgCurrSettleCnt)					// If the gas gauge is given enough time to settle it's average to slow charge current
			{
				
				int16_t max_slow_curr = (pEEP->SlowChrgCurr * 6)/5;
				if (Gauge.GGInstCurr > max_slow_curr)			// If slow charging current is not within the specified tolerance
				{
					if (++pBatt->NearlyChrgedCnt > 3)
						return SLOW_CURR_FAULT;					// Return with slow current fault
				}
				else
					pBatt->NearlyChrgedCnt = 0;
				AutoAdjustPWMI(pEEP->SlowChrgCurr);			// Dynamically adjust PWM I value in order to tally with slow charge current
			}
			else
				pBatt->AvgCurrSettleCnt--;					// Decrement average current settle counter

			if ((Gauge.Volt > pEEP->SlowFastVolt) && (pBatt->BattType != BATT_TYPE_DEAD))			// If battery voltage has reached to slow fast threshold
				pBatt->ChrgState = START_FAST_CHRG;			// Switch to fast charge

			SetChrgLEDPat(CHARGING_ON_PROGRESS);			// Set charging on progress LED pattern
			break;

		case START_FAST_CHRG:
			pBatt->ChrgTimeSecs = pEEP->FastChrgMins * 60;	// Set fast charge timeout
			pBatt->FullyChrgedCnt  = 0;						// Clear fully charged counter
			pBatt->NearlyChrgedCnt = 0;						// Clear nearly charged counter
			pBatt->FastChrgCurr = pEEP->FastChrgCurr;		// Set the fast charge current from the EEPROM
			pBatt->MiscFlags |= NEW_FAST_CURR_bm;			// Set the flag to indicate we have new fast current to set
			pBatt->ChrgState = DO_FAST_CHRG;				// Set next charger state
			PORTA_OUTSET = (CMOD0_bm << curr_slot);			// CMOD High
			// No "break" here

		case DO_FAST_CHRG:
			SetChrgLEDPat(CHARGING_ON_PROGRESS);			// Set the charging on progress LED pattern
			// No "break" here
		
		case CHRG_NEARLY_DONE:
			// ------------- Calibrate the toaster for current ----------------------------------
			if ((A2D_CALIB_CURR == Gauge.AvgCurr) && (pBatt->MiscFlags & FIXED_PWMI_bm))
			{
				int16_t i_err = Gauge.AvgCurr - Gauge.InstCurr;
				if (i_err < 0)
					i_err = -i_err;
				if (i_err > 10)
				{
					A2D_P *pa2d;
					pa2d = (A2D_P*)&A2D + curr_slot;
					
					uint16_t i_sense_const = ((uint32_t)Gauge.AvgCurr * pa2d->ISenseConst)/Gauge.InstCurr;	// Recalc the new constant for current sense
					if (i_sense_const != pa2d->ISenseConst)						// If the new value is different from old value
					{
						uint8_t addr = (curr_slot * 4) + CALIB_I_OFFSET;
						pa2d->ISenseConst = i_sense_const;						// Update the global storage
						WriteEEPData(addr, 2, (uint8_t*)&pa2d->ISenseConst);	// Store it in the EEPROM of the micro
					}
				}
			}	
			// ------------------------------------------------------------------------------------
			// ------------ Dynamically adjust fast charge current --------------------------------
			{
				uint16_t fast_curr = pEEP->FastChrgCurr;	// Assume fast charge current specified in EEPORM data
				
				//----------- Adjust the battery charge-up voltages according to temperature ---------------------------------
				if ((BATT_STATUS_ZQ3 == pBatt->BattType) || (BATT_TYPE_RLX1 == pBatt->BattType))
				{
					uint16_t chrgup_volt = pEEP->ChrgupVolt;
				
					if (pBatt->Tmp < pEEP->MaxTemp1)
					{
						chrgup_volt = pEEP->ChrgupVolt1;
						fast_curr = pEEP->FastChrgCurr1;
					}
					else
					{
						if (pBatt->Tmp < pEEP->MaxTemp2)
						{
							chrgup_volt = pEEP->ChrgupVolt2;
							fast_curr = pEEP->FastChrgCurr2;
						}
						else
						{
							if (pBatt->Tmp >= pEEP->MaxTemp3)
							{
								chrgup_volt = pEEP->ChrgupVolt3;
								fast_curr = pEEP->FastChrgCurr3;
							}
						}
					}
					if (chrgup_volt != pEEP->ChrgupVolt)
					{
						pEEP->ChrgupVolt = chrgup_volt;
						SetChrgupVoltage();
					}
				}
				//------------------------------------------------------------------------------------------------------------
					
				if (fast_curr > SYS.MaxFastChrgCurr)		// If the fast charge current specified in battery EEPROM is greater than the capabilities
					fast_curr = SYS.MaxFastChrgCurr;		// of the power supply/fuse, reduce the charge current estimated by dynamic current algorithm
				
				// -------- Reduce Fast charge current as battery temperature rises above 37'C ----------------------------
				if ((pBatt->Tmp > 37) && (fast_curr > FAST_CURR_ABOVE_37C))		// If battery temperature is above 37'C
					fast_curr = FAST_CURR_ABOVE_37C;							// Reduce the charging current to 1.5 Amps
					
				if ((pBatt->Tmp > 40) && (fast_curr > FAST_CURR_ABOVE_40C))		// If battery temperature is above 40'C
					fast_curr = FAST_CURR_ABOVE_40C;							// Reduce the charging current to 0.9 Amps
				
				if ((pBatt->Tmp > 42) && (fast_curr > FAST_CURR_ABOVE_42C))		// If battery temperature is above 42'C
					fast_curr = FAST_CURR_ABOVE_42C;							// Reduce the charging current to 0.5 Amps
					
				if (fast_curr > MAX_FAST_CHRG_CURR)
					fast_curr = MAX_FAST_CHRG_CURR;
				// --------------------------------------------------------------------------------------------------------
				
				if (fast_curr != pBatt->FastChrgCurr)		// If there is a change in fast charge current
				{
					pBatt->FastChrgCurr = fast_curr;		// Apply it
					pBatt->MiscFlags |= NEW_FAST_CURR_bm;
				}
			}
			
			if (pBatt->MiscFlags & NEW_FAST_CURR_bm)		// If there is a change in fast charge current
			{
				pBatt->MiscFlags &= ~(FIXED_PWMI_bm | NEW_FAST_CURR_bm);// Clear pwmi fixed flag in case it was set during slow charge
				pBatt->PWMSettleCnt = 0;
				pBatt->AvgCurrSettleCnt = FAST_SETTLE_TIME;	// Give some time to settle the gas gauge on new current
				SetPWMI(curr_slot, pBatt->FastChrgCurr);	// Set PWM I value corresponding to new fast charge current
			}
			
			//------- Auto adjust PWM V to ensure PP+ batteries are charged all the way up to CHARGE_UP_VOLTAGE ----------
			{
				int8_t pwm_v = 0;								// Start with no adjustment
				if ((Gauge.Volt > (pEEP->ChrgupVolt - 200)) &&  (Gauge.Volt < (pEEP->ChrgupVolt - 5)))	// If the gauge voltage is less then charge up voltage
				{
					// And charging current drops down to 80% of fast charge current and previous change is settled
					int16_t curr = pBatt->FastChrgCurr/5;
					if (curr < (pEEP->ChrgTermCurr + 100))
						curr = (pEEP->ChrgTermCurr + 100);
					if ((Gauge.GGInstCurr < curr) && (!pBatt->AvgCurrSettleCnt))
						pwm_v = 1;								// Increase the value by 1	
				}
				else
				{
					if (Gauge.Volt > (pEEP->ChrgupVolt-3))		// If gauge voltage is higher than charge up voltage
						pwm_v = -1;								// Decrease the value by 1
				}
					
				if (pwm_v)										// If PWM V adjustment is pending
				{
					uint16_t new_pwm_v = GetPWMV(curr_slot) + pwm_v;	// Calculate the new PWM V value
					if (new_pwm_v < pBatt->MaxPWMV)						// If new value is lesser than max limit
					{
						SetPWMV(curr_slot, new_pwm_v);					// Apply new PWM V value
						if (pwm_v > 0)									// If the change is towards high side
							pBatt->AvgCurrSettleCnt = PWMV_SETTLE_TIME;	// Give little time to settle things
					}
				}
			}
			//------------------------------------------------------------------------------------------------------------

			// -------- Check for fast charge timeout --------
			if (!pBatt->ChrgTimeSecs)						// If fast charge time is elapsed
				return FAST_TIME_FAULT;						// Return with fast time error

			// -------- Check for fast current fault ---------
			if (!pBatt->AvgCurrSettleCnt)					// If the gas gauge is given enough time to settle it's average to new fast charge current
			{// Yes
				int16_t max_fast_curr = (pBatt->FastChrgCurr * 6)/5;
				if (Gauge.GGInstCurr > max_fast_curr)		// If fast charging current is not within the specified tolerance
					return FAST_CURR_FAULT;					// Return with fast current fault
				AutoAdjustPWMI(pBatt->FastChrgCurr);		// Dynamically adjust PWM I value in order to tally with fast charge current
			}
			else
				pBatt->AvgCurrSettleCnt--;					// Decrement average current settle counter

			// -------- Check for charge nearly done ---------
			if (Gauge.AvgCurr < pEEP->NearlyDoneCurr)		// if charging current is less than nearly charged current
			{
				if (pBatt->NearlyChrgedCnt < _10_SECS)		// if the nearly charged counter is less than 10 seconds
					pBatt->NearlyChrgedCnt++;				// Keep incrementing the counter
				else
				{
					//SetChrgLEDPat(CHARGING_COMPLETE);		// Set charged nearly done LED pattern (same as fully charged pattern)
					pBatt->ChrgState = CHRG_NEARLY_DONE;	// Update the charger state as nearly done
				}
			}
			else
				pBatt->NearlyChrgedCnt = 0;					// Reset the nearly charged counter if charging current is equal or beyond the nearly charge current

			// ------- Check for fully charge condition based on charge termination current --------------
			
			// If the difference between battery voltage and charge up voltage is less than 100mV and charging current is less than charge termination current
			if (!(Gauge.Status[LOW_BYTE] & FC_bm))			// If fully charged (FC) bit is not set yet
			{
				if ((Gauge.Volt > (pEEP->ChrgupVolt - _100mV)) && (Gauge.AvgCurr < pEEP->ChrgTermCurr))
					pBatt->FullyChrgedCnt++;				// Increment the fully charge counter
				else
					pBatt->FullyChrgedCnt = 0;				// Reset the counter if any of the above condition is break
			}
			
			// ------------------ Check for fully charged condition -----------------------
			if (((Gauge.Status[LOW_BYTE] & FC_bm) && (pBatt->SOC >((pEEP->RechrgThreshold) +2)) ) || (pBatt->FullyChrgedCnt > _80_SECS))
			{
				ShutdownCharger();						// Turn off the charger 
				pBatt->ChrgState = FULLY_CHARGED;		// Update the charger state
				pBatt->ChrgTimeSecs = 0;				// Clear fast charge timeout
				pBatt->NearlyChrgedCnt = _10_SECS;		// Because we need to calibrate A2D after 10 seconds
				SetChrgLEDPat(CHARGING_COMPLETE);		// Set charged done LED status here	
			}
			break;

		case FULLY_CHARGED:
			SetChrgLEDPat(CHARGING_COMPLETE);			// Set charged done LED status here
			
			if (pBatt->SOC <= pEEP->RechrgThreshold)	// If the battery soc drops below recharge threshold
				pBatt->ChrgState = REQUEST_TO_CHRG;		// Recharge the battery
					
			// A2D auto calibration routine to compensate for A2D gain error
			if (pBatt->NearlyChrgedCnt)
			{
				if (--pBatt->NearlyChrgedCnt == 0)		// If 10 seconds elapsed
				{
					A2D_P *pa2d;
					pa2d = (A2D_P*)&A2D + curr_slot;
					uint8_t addr = curr_slot * 4;
					int16_t a2d_gain_err;
					a2d_gain_err = pa2d->BattVSense - (int16_t)Gauge.Volt;	// Calculate the gain error (in mVolts)
					if (a2d_gain_err < 0)				// If the gain error is negative
						a2d_gain_err = -a2d_gain_err;	// Make it positive
					if (a2d_gain_err > MAX_GAIN_ERROR)	// If the gain error is beyond 2 LSB's
					{
						pa2d->VSenseConst = ((uint32_t)Gauge.Volt * pa2d->VSenseConst)/pa2d->BattVSense;	// Recalc the new gain correction factor
						WriteEEPData(addr, 2, (uint8_t*)&pa2d->VSenseConst);	// Store it in the EEPROM of the micro
					#ifdef _DEBUG_MSGS_
						SendRS232('e');
					#endif
					}
				}
			}
			break;

		case CHRG_PAUSE:								// Comes here during battery calibration (Not used in Jasper)
			SetChrgLEDPat(CHARGING_ON_PROGRESS);		// Keep the LED blinking going
			break;

		case BAD_TMP:									// Comes here if battery temperature goes beyond the safe limits
			SetChrgLEDPat(CHARGING_ERROR);				// Set error blinking pattern
			if ((pBatt->Tmp >= pEEP->ColdOn ) && (pBatt->Tmp <= pEEP->HotOn ))// If the battery temperature comes back to safe region
			{
				pBatt->ChrgState = REQUEST_TO_CHRG;		// Turn on the charger
			}
			break;	
	}
	return NO_ERROR;
}

//----------------------------------------------------------------------
//
// Prototype:	SetChrgupVoltage
//
// Description:	Routine to set PWM V registers as per battery charge up volatge 
//
// Parameters:	None
//
// Returns:		None
//
// Notes:
//----------------------------------------------------------------------
void SetChrgupVoltage(void)
{
	// ----- Set the charge-up voltage ----
	uint8_t offset = ((pEEP->ChrgupVolt - MIN_CHRG_UP_VOLT)/VOLT_STEP_SIZE);	// offset is always <= 40
	offset <<= 1;								// Multiply the offset by 2
	uint16_t pwmv = pgm_read_word(PSTR(PWMV_LOOKUP_TABLE) + offset);
	SetPWMV(PORTR_OUT, pwmv);					// Set PWM V value corresponding to charge up voltage
	offset += 4;								// Jump two steps above
	if (offset> 160)
		offset = 160;
	pBatt->MaxPWMV = pgm_read_word(PSTR(PWMV_LOOKUP_TABLE) + offset);
}
//----------------------------------------------------------------------
//
// Prototype:	AutoAdjustPWMI
//
// Description:	Routine to automatically adjust PWM_I according to slow/fast current specified in the battery EEPROM
//
// Parameters:	set_current: current to be set
//
// Returns:		None
//
// Notes:
//----------------------------------------------------------------------
void AutoAdjustPWMI(int16_t set_current)
{
	if (pBatt->MiscFlags & (FIXED_PWMI_bm | MANUAL_PWMI_bm))	// If PWMI manual adjustment is enabled
		return;													// Yes, just return;
		
	if ((!SYS.OneSecCnt) || (SYS.OneSecCnt % 3))	// These two lines are added just to slow down the auto adjust process as it takes some time to settle the Gauge.GGInstCurr
		return;				
	
	uint8_t slot = PORTR_OUT;
	int16_t curr_err = Gauge.GGInstCurr - set_current;	// Calculate the error
	int16_t max_steps = set_current/5;				// Allow 20% percent of the theoretical value in either side
	int16_t pwm_i = *PWMIReg(slot);					// Read the current PWMI value
	
	//---------- Handle bigger errors first ---------------------------------
	if (curr_err > 50)								// If error is too big on positive side
	{
		pwm_i -= curr_err;							// fix it immediately
		if (pwm_i < MIN_CHRG_CURR)
			pwm_i = MIN_CHRG_CURR;
		*PWMIReg(slot) = pwm_i;
		pBatt->AvgCurrSettleCnt = FAST_SETTLE_TIME;	// Give some time to settle the gas gauge on new current
		return;
	}
	//-----------------------------------------------------------------------

	if (curr_err > 0)								// If actual current is higher than the set value
	{
		if ((pwm_i > (set_current - max_steps)) && (pwm_i > MIN_CHRG_CURR))	// If present PWMI value can be further decreased?
			pwm_i--;														// Yes, decrease the PWMI value by 1
	}
	
	if (curr_err < 0)	// If actual current is lower than the set value
	{
		if ((pwm_i < (set_current + max_steps)) && (pwm_i < MAX_PWMI_CURR))	// If present PWMI value can be further increased?
			pwm_i++;														// Yes, increase the PWMI value by 1
		
		if (!pBatt->PWMSettleCnt)
			pBatt->Volt_PWM_Off = curr_err;
		
		if (++pBatt->PWMSettleCnt > 15)
		{
			if (curr_err <= pBatt->Volt_PWM_Off)
			{
				pBatt->MiscFlags |= FIXED_PWMI_bm;
				//Print_P(PSTR("\r\nPWMI Fixed"));
				//SendHex(slot);
			}
			pBatt->PWMSettleCnt = 0;
		}
	}

	*PWMIReg(slot) = pwm_i;						// Set the adjusted PWMI
}

//----------------------------------------------------------------------
//
// Prototype:	ShutdownCharger
//
// Description:	Routine to turn off the charger in current slot
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void ShutdownCharger(void)
{
	uint8_t curr_slot = PORTR_OUT;
	PORTA_OUTSET = (CHG_DIS0_bm << curr_slot);	// Disable the charger
	PORTA_OUTCLR = (CMOD0_bm << curr_slot);		// Switch to pre-charge mode
	SetPWMI(curr_slot, 0);						// Turn off current PWM
	SetPWMV(curr_slot, 0);						// Turn off voltage PWM
}

//----------------------------------------------------------------------
//
// Prototype:	HandleOverCurrInt
//
// Description:	Routine to handle over current situation triggered by ICH interrupt
//
// Parameters:	None
//
// Returns:		None
// 
// Notes:
//----------------------------------------------------------------------
void HandleOverCurrInt(void)
{
	PORTA_OUTSET = CHG_DIS_gm;			// Disable all chargers
	DISABLE_OVER_CURR_INT;				// Disable over current interrupt
	SYS.OverCurrRecoveryCnt = OVER_CURR_RECOVERY_TIME;	// Try to recover after 5 seconds (Just to pass ESD test)
#ifdef _ALWAYS_RECOVER_
	if (SYS.OverCurrOneSecCnt)			// If over current condition repeats (Re-trigger within 1 second after enabling ICH interrupt)
	{
		SYS.OverCurrOneSecCnt = 0;		// Clear 1 second counter
		SYS.OverCurrRepeatCnt--;		// Decrement the repeat count by 1
	}
	else
#endif
		SYS.OverCurrRepeatCnt = MAX_OVR_CURR_EVENTS;	// This happens at the very first interrupt

	SendHex(SYS.OverCurrRepeatCnt);
	Print_P(PSTR(OVER_CURR_MSG));		// Print "Over Curr-nn". Other purpose is to loop through main() few times via uart TXD interrupt
}

//----------------------------------------------------------------------
//
// Prototype:	SIGNAL (PORTB_INT0_vect)
//
// Description:	Charge over current ISR
//
// Parameters:	None 
//
// Returns:		None
// 
// Notes: 	Once triggered, a hard reset is required to escape from this condition
//
//----------------------------------------------------------------------
SIGNAL (PORTB_INT0_vect)
{
	HandleOverCurrInt();
}