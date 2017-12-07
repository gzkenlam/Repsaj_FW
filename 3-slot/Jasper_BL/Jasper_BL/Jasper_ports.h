//-----------------------------------------------------------------------------
// FILENAME:  Jasper_Ports.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Jasper charger Micro GPIO ports/directions and pullup definitions are in this file
//
// %IF Zebra_Internal
//
// NOTES:    
//
// AUTHOR:   		Wasath Mudalige
// CREATION DATE: 	03/25/2016
// DERIVED FROM: 	New file
//
// EDIT HISTORY:
//
//
//
// %End 
//-----------------------------------------------------------------------------
#ifndef JASPER_PORTS_H
#define JASPER_PORTS_H

#include <avr/io.h>

// Port A defines
//	Pin	Dir	Description
//	PA0	In	Current sense (All Slots)
//	PA1	In	Temperature sense (All Slots)
//	PA2	Out	CMOD (Slot0)
//	PA3	Out	CMOD (Slot1)
//	PA4	Out	CMOD (Slot2)
//	PA5	Out Chrg_Disable (Slot0)
//	PA6	Out Chrg_Disable (Slot1)
//	PA7	Out	Chrg_Disable (Slot2)

// Direction and Data
#define DATA_PORTA	0xC0
#define DIR_PORTA	0xFC

// I/O controls
#define PIN0CTRL_PORTA	PORT_ISC_INPUT_DISABLE_gc
#define PIN1CTRL_PORTA	PORT_ISC_INPUT_DISABLE_gc
#define PIN2CTRL_PORTA	PORT_OPC_TOTEM_gc
#define PIN3CTRL_PORTA	PORT_OPC_TOTEM_gc
#define PIN4CTRL_PORTA	PORT_OPC_TOTEM_gc
#define PIN5CTRL_PORTA	PORT_OPC_TOTEM_gc
#define PIN6CTRL_PORTA	PORT_OPC_TOTEM_gc
#define PIN7CTRL_PORTA	PORT_OPC_TOTEM_gc

// Charger Disable
#define CHG_DIS0_bm		PIN5_bm
#define CHG_DIS1_bm		PIN6_bm
#define CHG_DIS2_bm		PIN7_bm
#define CHG_DIS_gm		(CHG_DIS0_bm | CHG_DIS1_bm | CHG_DIS2_bm)

#define CMOD0_bm		PIN2_bm
#define CMOD1_bm		PIN3_bm
#define CMOD2_bm		PIN4_bm
#define CMOD_LOW_ALL_CHRG_SLOTS		PORTA_OUTCLR = (CMOD0_bm | CMOD1_bm | CMOD2_bm)

// Port B defines
//	Pin	Dir	Description
//	PB0	In	Voltage Sense (Slot0)
//	PB1	In	Voltage Sense (Slot1)
//	PB2	In	Error*
//	PB3	In	Voltage Sense (Slot2)

// Direction and Data
#define DATA_PORTB	0x00
#define DIR_PORTB	0x00

// I/O controls
#define PIN0CTRL_PORTB	PORT_ISC_INPUT_DISABLE_gc
#define PIN1CTRL_PORTB	PORT_ISC_INPUT_DISABLE_gc
#define PIN2CTRL_PORTB	PORT_OPC_PULLUP_gc
#define PIN3CTRL_PORTB	PORT_ISC_INPUT_DISABLE_gc

// Over current interrupt
#define OVER_CURR_bm					PIN2_bm
#define OVER_CURR_PORT					PORTB_IN
#define CLEAR_PENDING_OVER_CURR_INTS	PORTB_INTFLAGS = PORT_INT0IF_bm
#define ENABLE_OVER_CURR_INT			PORTB_INTCTRL = PORT_INT0LVL_HI_gc
#define DISABLE_OVER_CURR_INT			PORTB_INTCTRL = PORT_INT0LVL_OFF_gc

// Port C defines
//	Pin	Dir	Description
//	PC0	I/O	Battery I2C SDA
//	PC1	I/O	Battery I2C SCL
// 	PC2	In	Debug RX
//	PC3	Out	Debug TX
//	PC4	Out	PWM_I (Slot0)
//	PC5	Out	PWM_I (Slot1)
//	PC6	Out	PWM_I (Slot2)
//	PC7	Out	Reserved

// Direction and Data
#define DATA_PORTC	0x08
#define DIR_PORTC	0xF8

// I/O controls
#define PIN0CTRL_PORTC	PORT_OPC_TOTEM_gc
#define PIN1CTRL_PORTC	PORT_OPC_TOTEM_gc
#define PIN2CTRL_PORTC	PORT_OPC_PULLUP_gc
#define PIN3CTRL_PORTC	PORT_OPC_TOTEM_gc
#define PIN4CTRL_PORTC	PORT_OPC_TOTEM_gc
#define PIN5CTRL_PORTC	PORT_OPC_TOTEM_gc
#define PIN6CTRL_PORTC	PORT_OPC_TOTEM_gc
#define PIN7CTRL_PORTC	PORT_OPC_TOTEM_gc

// UART receive
// UART receive
#define DEBUG_PORT		PORTC_IN
#define DEBUG_RXD_bm	PIN2_bm

// UART Receive pin change interrupt defs
#define CLEAR_PENDING_RX_PIN_CHANGE_INTS	PORTC_INTFLAGS |= PORT_INT1IF_bm
#define ENABLE_RX_PIN_CHANGE_INT			PORTC_INTCTRL  |= PORT_INT1LVL_MED_gc
#define DISABLE_RX_PIN_CHANGE_INT			PORTC_INTCTRL  &= ~PORT_INT1LVL_gm

// Port D defines
//	Pin	Dir	Description
//	PD0	Out	LED Red   (Slot0)
//	PD1	Out	LED Green (Slot0)
//	PD2	Out	LED Red   (Slot1)
//	PD3	Out	LED Green (Slot1)
//	PD4	Out LED Red   (Slot2)
//	PD5	Out LED Green (Slot2)
//	PD6	Out	Reserved
//	PD7	Out Reserved

// Direction and Data
#define DATA_PORTD	0x00
#define DIR_PORTD	0xFF

// I/O controls
#define PIN0CTRL_PORTD	PORT_OPC_TOTEM_gc
#define PIN1CTRL_PORTD	PORT_OPC_TOTEM_gc
#define PIN2CTRL_PORTD	PORT_OPC_TOTEM_gc
#define PIN3CTRL_PORTD	PORT_OPC_TOTEM_gc
#define PIN4CTRL_PORTD	PORT_OPC_TOTEM_gc
#define PIN5CTRL_PORTD	PORT_OPC_TOTEM_gc
#define PIN6CTRL_PORTD	PORT_OPC_TOTEM_gc
#define PIN7CTRL_PORTD	PORT_OPC_TOTEM_gc

// Charge LEDs defs
#define RED_LED_bm		PIN0_bm
#define GRN_LED_bm		PIN1_bm
#define CHRG_LEDS_gm	(RED_LED_bm | GRN_LED_bm)
#define ALL_LEDS_gm		(PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm)

// Port E defines
//	Pin	Dir	Description
//	PE0	Out	PWM_V (Slot0)
//	PE1	Out	PWM_V (Slot1)
//	PE2	Out	PWM_V (Slot2)
//	PE3	Out	SCL Buf*

// Direction and Data
#define DATA_PORTE	0x00
#define DIR_PORTE	0x0F

// I/O controls
#define PIN0CTRL_PORTE	PORT_OPC_TOTEM_gc
#define PIN1CTRL_PORTE	PORT_OPC_TOTEM_gc
#define PIN2CTRL_PORTE	PORT_OPC_TOTEM_gc
#define PIN3CTRL_PORTE	PORT_OPC_TOTEM_gc

// I2C Clock Buffer
#define nCLK_BUF			PIN3_bm
#define ENABLE_CLK_BUF		PORTE_OUTCLR = nCLK_BUF
#define DISABLE_CLK_BUF 	PORTE_OUTSET = nCLK_BUF

// Port R defines
//	Pin	Dir	Description
//	PR0	In	Charger Select0
//	PR1	In  Charger Select1

// Direction and Data
#define DATA_PORTR	0x00
#define DIR_PORTR	0x03

// I/O controls
#define PIN0CTRL_PORTR	PORT_OPC_TOTEM_gc
#define PIN1CTRL_PORTR	PORT_OPC_TOTEM_gc

// Function prototypes
void SetupPorts(void);
void Init_ExtInts(void);
void Deinit_ExtInts(void);

#endif // Rogue_Ports.h
