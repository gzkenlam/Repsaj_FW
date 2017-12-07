/* 
 * File:   misc.h
 * Author: KLam
 *
 * Created on November 18, 2016, 2:37 PM
 */

#ifndef MISC_H
#define	MISC_H

#include <mcp19118.h>
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "i2c.h"
#include "interrupt_manager.h"
#include "regulator.h"   

#ifdef	__cplusplus
extern "C" {
#endif

//#define DEBUG_ON
    
#define MCP19118
    
#define MAIN_VER    0x10
#define SUB_VER     0x06
    
#define CAL_BASE_ADDR   (0xE80)    

/* GPIO Definition */
#ifdef MCP19118
#define GPIOA_SCL_PIN       0x80        //GPA7 SCL1
#define GPIOA_GPA6_PIN     0x40        //GPA6 
#define GPIOA_MCLR_PIN      0x20       //GPA5  MCLR
#define GPIOA_GPA4_PIN      0x10        //GPA4 
#define GPIOA_THERM_PIN     0x08        //GPA3
#define GPIOA_WDFEED_PIN    0x04        //GPA2 output
#define GPIOA_MODE_PIN      0x02        //GPA1    
#define GPIOA_NBATT_PIN     0x01        //GPA0


#define GPIOB_CHGSW_PIN     0x04        //GPB2 output
#define GPIOB_VBATT_PIN     0x02        //GPB1
#define GPIOB_SDA_PIN       0x01        //GPB0
    
#define CHGSW   PORTBbits.GPB2
#define WDFEED  PORTAbits.GPA2
#define GPA4PIN PORTAbits.GPA4
#define GPA4PIN_SetHigh()            do { GPA4PIN = 1; } while(0)
#define GPA4PIN_SetLow()             do { GPA4PIN = 0; } while(0)
#define GPA4PIN_Toggle()             do { GPA4PIN = ~GPA4PIN; } while(0)    
#define CHGSW_SetHigh()            do { CHGSW = 1; } while(0)
#define CHGSW_SetLow()             do { CHGSW = 0; } while(0)
#define CHGSW_Toggle()             do { CHGSW = ~CHGSW; } while(0)
#define WDFEED_SetHigh()            do { WDFEED = 1; } while(0)
#define WDFEED_SetLow()             do { WDFEED = 0; } while(0)
#define WDFEED_Toggle()             do { WDFEED = ~WDFEED; } while(0)
    
#else
    
#define GPIOA_THERM_PIN 0x08
#define GPIOA_LED2_PIN  0x01
#define GPIOA_WD_PIN    0x04
#define GPIOA_NBATT_PIN 0x02
#define GPIOA_MCLR_PIN  0x20
#define GPIOA_SW1_PIN   0x40
#define GPIOA_SW2_PIN   0x10
#define GPIOA_SCL_PIN   0x80
    
#define GPIOB_CHGON_PIN 0x80
#define GPIOB_LED1_PIN  0x04
#define GPIOB_VBATT_PIN 0x02
#define GPIOB_SDA_PIN   0x01

#endif

/*GPIO Operation*/
#ifdef MCP19118

#else
    
#define CHGON   PORTBbits.GPB7
#define LED1    PORTBbits.GPB2
#define LED2    PORTAbits.GPA0
#define WD      PORTAbits.GPA2
#define SW1     PORTAbits.GPA6
#define SW2     PORTAbits.GPA4
#endif

/* u16Timer_Slot_Counter Bit Definition*/
#define   TIME_SLOT_COUNTER_BIT_00   0x01
#define   TIME_SLOT_COUNTER_BIT_01   0x02
#define   TIME_SLOT_COUNTER_BIT_02   0x04
#define   TIME_SLOT_COUNTER_BIT_03   0x08
#define   TIME_SLOT_COUNTER_BIT_04   0x10
#define   TIME_SLOT_COUNTER_BIT_05   0x20
#define   TIME_SLOT_COUNTER_BIT_06   0x40
#define   TIME_SLOT_COUNTER_BIT_07   0x80
    
/* Adc Channel value */
#define ADC_SAMPLE_RATE 16
#define SHIFT_BITS      2
#define SHIFT_MOD       (0x01<<SHIFT_BITS)
#define ADC_MUX_VIN     (0x00 << 2)     //VIN_ANA 1/13 of Vin
#define ADC_MUX_VBAT    (0x14 << 2)     //GPB1  0.217*VBAT
#define ADC_MUX_IOUT    (0x05 << 2)     //Vreg load voltage
#define ADC_MUX_THERM   (0x13 << 2)     //GPA3 Thermal    
#define ADC_MUX_VREF    (0x01 << 2)     //reference of Vreg output
#define ADC_MUX_VBGR    (0x04 << 2)     //Band Gap reference
#define ADC_MUX_VZC     (0x07 << 2)     //Voltage for Zero Current
#define ADC_MUX_TMP     (0x12 << 2)     //Temperature
#define ADC_MUX_CRT     (0x06 << 2)     //inductor current    

   
    
//struct ADC_BUFFER shall be edited together with u8AdcChannel[], channels shall be aligned    
typedef struct{
    uint16_t    u16Vin;
    uint16_t    u16VBat;
    uint16_t    u16IOut;
    uint16_t    u16Therm;
}ADC_BUFFER;

extern  ADC_BUFFER  adc;

typedef struct{
    uint16_t    u16Calib800mA;
    uint16_t    u16CalibZeroA;
    uint16_t    u16Calib800mV;
    uint16_t    u16Calib8V4;
}CALIB_VALUE;

extern CALIB_VALUE cal;

extern uint8_t u8TimerSlotCounter;

extern uint16_t u16CommTimeOut;

void SYSTEM_Init(void);

void IO_Init(void);

void Cal_Init(void);

void Timer0_Init(void);

void ADC_Init(void);

void Timer0_ISR(void);

void Write_Flash(uint16_t addr, uint8_t * Pdata);

uint16_t Read_Flash(uint16_t addr);

void ADC_Pooling(void);




#ifdef	__cplusplus
}
#endif

#endif	/* MISC_H */

