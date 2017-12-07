/* 
 * File:   misc.h
 * Author: KLam
 *
 * Created on November 23, 2016, 6:32 PM
 */

#ifndef MISC_H
#define	MISC_H

#include "global.h"

#ifdef	__cplusplus
extern "C" {
#endif

/* RTOS lite */
#define	TIME_SLOT_COUNTER_BIT_00	0x01
#define	TIME_SLOT_COUNTER_BIT_01	0x02
#define	TIME_SLOT_COUNTER_BIT_02	0x04
#define	TIME_SLOT_COUNTER_BIT_03	0x08
#define	TIME_SLOT_COUNTER_BIT_04	0x10
#define	TIME_SLOT_COUNTER_BIT_05	0x20
#define	TIME_SLOT_COUNTER_BIT_06	0x40
#define	TIME_SLOT_COUNTER_BIT_07	0x80
    
/* LED Status */
#define LED_OFF     0x01
#define REMOTE_CTRL 0x02
#define SOLID_RED   0x03
#define SOLID_GREEN 0x04
#define SOLID_AMBER 0x05
#define FBLINK_RED  0x06
#define BURST_AMBER 0x07
#define BURST_GREEN 0x08
    
/* LED PWM */
#define LED_OFF_PWM         0
#define LED_GRN_PWM         (20*5)
#define LED_AMBER_GRN_PWM   (18*5) 
#define LED_AMBER_RED_PWM   (70*5)
#define LED_RED_PWM         (40*5)
#define LED_BURST_AMBER_GRN_PWM 130
#define LED_BURST_AMBER_RED_PWM 450
#define LED_BURST_GRN_PWM   200

extern uint8_t u8LEDStatus;
    
extern uint8_t  u8TimerSlotCounter; 

extern uint16_t u16CommTimeOut;

extern  uint8_t u8LEDOverridden;

void Set_LED(uint8_t u8led);
void LED_Handle(void);       //Call every 32ms
void Lite_RTOS_Timer(void);   //Call in Timer every 1ms
void Delay_Msec(uint16_t Msec);
void MCP_I2C_Check(void);
void Value_Init(void);
void ResetDevice();
#ifdef	__cplusplus
}
#endif

#endif	/* MISC_H */

