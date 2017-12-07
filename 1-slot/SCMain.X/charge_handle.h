/* 
 * File:   charge_handle.h
 * Author: KLam
 *
 * Created on November 23, 2016, 7:00 PM
 */

#ifndef CHARGE_HANDLE_H
#define	CHARGE_HANDLE_H

#include "global.h"
#include <stdio.h>
#include <string.h>

#ifdef	__cplusplus
extern "C" {
#endif

/* Default Value of Charger */
#define INPUT_THRESHOLD             680      //(10*4095/5/13)

/* batt without fg is not supported
#define DC_8V_50mV          160         //8000/50
#define NTC_COOL_START_ADC  0xf8C       //37K ~ -2C;
#define NTC_HOT_START_ADC   0x07F       //5.07K 41C;
#define NTC_COOL_CHARGE_ADC NTC_COOL_START_ADC
#define NTC_HOT_CHARGE_ADC  0x64       //4.1K  46C;
#define MAX_CHARGE_TIME     8       //8hrs
#define PRECHARGE_TIMEOUT           1    //60*20
*/
    
/* Charge Management Status */
#define INIT_CHARGE     0x0100
#define CALIBING        0x0200
#define WAIT_FOR_CALIB  0x0400
#define PRE_CHARGE      0x0800
#define CC_CHARGE       0x1000
#define CV_CHARGE       0x2000
#define FULL_CHARGE     0x4000
#define USER_INTERACT   0x8000
//#define REMOTE_MODE     0x0000

extern  int16_t s16MaxChargeCurr;
extern  int16_t s16MaxChargeVolt;
extern  uint8_t u8CalibStep;    

extern  uint16_t u16SecCounter;
extern  uint16_t    u16SlowTimeSec;
extern  uint8_t u8SlowTimeHr;
extern  uint16_t    u16FastTimeSec;
extern  uint8_t  u8FastTimeHr;
extern uint16_t u16TotalChargeSec;

extern  uint16_t u16ChargerStatus;

extern  uint8_t u8MCPSendDone;

void User_Yes_Interface(void);
void User_No_Interface(void);
void ChargeTimer(void);
void Send_MCPCmd(uint8_t MCPCmd, uint16_t Val1, uint16_t Val2);
void Charger_Handle(void);

    

    
#ifdef	__cplusplus
}
#endif

#endif	/* CHARGE_HANDLE_H */

