/**
  Generated Main Source File

  Company: Zebra Technologies

  File Name:
    main.c

  Summary:
    This is the main file for single battery charger main, 
    microcontroller : PIC16F18326
* ------------------------------------------------------------------------------
 * Rev 1.17 2017-9-5
 1. ZQ3/5 reading CF from MAC;
* ------------------------------------------------------------------------------
Rev 1.15 2017-8-29
1. Add ZQ5 BattMode() to support CF detection
* ------------------------------------------------------------------------------
 * Rev 1.13 2017-7-28
 * 1. extend calibrate time
Rev 1.12 2017-6-28
Modification 
1. Remove Cycle count >600, error
2. Add MCP FW version display
--------------------------------------------------------------------------------  
Rev 1.112017-6-27
Modification 
1. Add Cycle count >600, error
-------------------------------------------------------------------------------- 
Rev 1.10 2017-5-25
 * Modification 
1. LED drive changed
2. Add decomission setting 
3. Add one more requirement for inserted full battery(SOC is > 92)
-------------------------------------------------------------------------------- 
Rev 1.9 2017-5-20
Modification
1.	Add Dead battery support in MCP19118
--------------------------------------------------------------------------------
Rev 1.8 2017-4-28
Modification
1.	Add Weak Pull up setting for I2C
--------------------------------------------------------------------------------
Rev 1.7 2017-4-27
Modification
1.	Work with Bootloader
2. Need modify configuration bit to PPS 1-way: OFF;
2. WDT switch to 
-------------------------------------------------------------------------------- 
Rev 1.6 2017-4-6
Modification
1.	Detecting a full-charged battery by Fuel Gauge reading;
2.  Add SOH detection
--------------------------------------------------------------------------------
Rev 1.4 2017-3-21
Modification
1.	Adding temperature tolerance for charging;
2.  Resolve LED blink rate error during charging;
3.  Resolve communication bug with MCP19118;
--------------------------------------------------------------------------------
Rev 1.3 2017-3-8
Modification
1.	Adding charge current limitation for Rolex battery;
--------------------------------------------------------------------------------
Rev 1.2 2017-2-23
Modification
2.	On MCP19118, adjust INUVP to 10.8V to resolve AC cut off charge back issue;
--------------------------------------------------------------------------------
Rev 1.1 2017-2-15
Modification
1.	Resolve problem of battery communication, Sanyo batteries;
2.	LED color adjust, yellow to amber;
3.	Charge current is limited to 2000mA;
--------------------------------------------------------------------------------
Rev 1.0 2017-1-5
Features
1.	Support batteries, QLn3/4, ZQ5, ZQ3(Authentication); 
2.	Rolex battery will be charged as QLn or ZQ3, depending on if ATECC508 data reading correctly for the new sample;
3.	Charge current is limited to 1850mA (hardware technically supports 2.5A);
4.	On the go FW upgrade is not supported 
Known Issues
1.	Short term(5-6s) Red Blink during normal charging period, root cause is communication with battery lost during 
    this period, impact is charge will be terminated in short while, charger can recover, it will be resolved by 
    hardware modification, I2C level shifter
* ------------------------------------------------------------------------------

*/

#include "global.h"

uint8_t u8DispSeq = 0;
uint8_t u8QuerySeq = 0;
uint16_t u16Temp;

/*
                         Main application
 */
void main(void)
{
    // initialize the device
    Value_Init();
    SYSTEM_Initialize();
    // When using interrupts, you need to set the Global and Peripheral Interrupt Enable bits
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();
    
    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();

    MCP_I2C_Check();
    Buffer_Init();
    Show_Version();
    Debug_Mode(DEBUG_CHARGE_STATUS);    
    while(1)
    {
        if(u8TimerSlotCounter & TIME_SLOT_COUNTER_BIT_01)
        {
            u8TimerSlotCounter ^= TIME_SLOT_COUNTER_BIT_01;
            /* Time_Slot_01 function here called every 2ms */
        }
        if(u8TimerSlotCounter & TIME_SLOT_COUNTER_BIT_02)
        {
            u8TimerSlotCounter ^= TIME_SLOT_COUNTER_BIT_02;
            /* Time_Slot_02 function here called every 4ms */			
        }
        if(u8TimerSlotCounter & TIME_SLOT_COUNTER_BIT_03)
        {
            u8TimerSlotCounter ^= TIME_SLOT_COUNTER_BIT_03;
            /* Time_Slot_03 function here called every 8ms */
        }
        if(u8TimerSlotCounter & TIME_SLOT_COUNTER_BIT_04)
        {
            u8TimerSlotCounter ^= TIME_SLOT_COUNTER_BIT_04;
            /* Time_Slot_04 function here called every 16ms */
        }
        if(u8TimerSlotCounter & TIME_SLOT_COUNTER_BIT_05)
        {
            u8TimerSlotCounter ^= TIME_SLOT_COUNTER_BIT_05;
            /* Time_Slot_05 function here called every 32ms */
            
        }
        if(u8TimerSlotCounter & TIME_SLOT_COUNTER_BIT_06)
        {
            u8TimerSlotCounter ^= TIME_SLOT_COUNTER_BIT_06;
            /* Time_Slot_05 function here called every 64ms */
            u8QuerySeq++;
            u8QuerySeq &= 0x03;
            switch (u8QuerySeq){
                case 0:{                    
                    MCP_CmdSend();
                    break;
                }
                case 1:{
                    u16Temp = u16ChargerStatus;     // add this read of u16ChargerStatus, unless the u16ChargerStatus will be modified 
                    Batt_Comm();
                    break;
                }
                case 2:{
                    BattDetect_DeJitter();
                    break;
                }
                case 3:
                default:{
                    USART_Handle();
                    Charger_Handle();
                    u8DispSeq++;
                    u8DispSeq &= 0x03;
                    if(!u8DispSeq){
                        Debug_Display();
                    }
                    break;
                }
            }
            
        }
        if(u8TimerSlotCounter & TIME_SLOT_COUNTER_BIT_07)
        {
            u8TimerSlotCounter ^= TIME_SLOT_COUNTER_BIT_07;
            /* Time_Slot_07 function here called every 128ms */
            
        }
    }
}

/**
 End of File
*/