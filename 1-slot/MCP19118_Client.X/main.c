/*
 * File:   main.c
 * Author: KLam
 *
 * This is main file for single battery charger, power management microcontroller: MCP19118
 * 
 * Created on November 18, 2016, 2:29 PM
 * 
 * VER. 1.6
 * 1. Adding version checking feature
 * 
 * 
 */



#include "misc.h"


void main(void) {
    SYSTEM_Init();
    INTERRUPT_PeripheralInterruptEnable();
    INTERRUPT_GlobalInterruptEnable();
    MCP_Regulation_Init();
    while(1){
        /* there is 500us in each slot */
        if(u8TimerSlotCounter & TIME_SLOT_COUNTER_BIT_01)   {
            u8TimerSlotCounter ^= TIME_SLOT_COUNTER_BIT_01;
            /* Time_Slot_01 function here called every 1ms */
            I2C_Data_Handler();
            ADC_Pooling();
        }
        if(u8TimerSlotCounter & TIME_SLOT_COUNTER_BIT_02)   {
            u8TimerSlotCounter ^= TIME_SLOT_COUNTER_BIT_02;
            /* Time_Slot_02 function here called every 2ms */            
        }
        if(u8TimerSlotCounter & TIME_SLOT_COUNTER_BIT_03)   {
            u8TimerSlotCounter ^= TIME_SLOT_COUNTER_BIT_03;
            /* Time_Slot_03 function here called every 4ms */
        }
        if(u8TimerSlotCounter & TIME_SLOT_COUNTER_BIT_04)   {
            u8TimerSlotCounter ^= TIME_SLOT_COUNTER_BIT_04;
            /* Time_Slot_04 function here called every 8ms */
        }
        if(u8TimerSlotCounter & TIME_SLOT_COUNTER_BIT_05)   {
            u8TimerSlotCounter ^= TIME_SLOT_COUNTER_BIT_05;
            /* Time_Slot_05 function here called every 16ms */
            Regulation_Volt();
            Regulation_Curr();
        }
        if(u8TimerSlotCounter & TIME_SLOT_COUNTER_BIT_06)   {
            u8TimerSlotCounter ^= TIME_SLOT_COUNTER_BIT_06;
            /* Time_Slot_06 function here called every 32ms */
            Status_Maintain();    
        }
        if(u8TimerSlotCounter & TIME_SLOT_COUNTER_BIT_07)   {
            u8TimerSlotCounter ^= TIME_SLOT_COUNTER_BIT_07;
            /* Time_Slot_07 function here called every 64ms */
        }
    }        
    return;
}
