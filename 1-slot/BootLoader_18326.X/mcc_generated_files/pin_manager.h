/**
  @Generated Pin Manager Header File

  @Company:
    Microchip Technology Inc.

  @File Name:
    pin_manager.h

  @Summary:
    This is the Pin Manager file generated using MPLAB(c) Code Configurator

  @Description:
    This header file provides implementations for pin APIs for all pins selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB(c) Code Configurator - 4.15
        Device            :  PIC16F18326
        Version           :  1.01
    The generated drivers are tested against the following:
        Compiler          :  XC8 1.35
        MPLAB             :  MPLAB X 3.40

    Copyright (c) 2013 - 2015 released Microchip Technology Inc.  All rights reserved.

    Microchip licenses to you the right to use, modify, copy and distribute
    Software only when embedded on a Microchip microcontroller or digital signal
    controller that is integrated into your product or third party product
    (pursuant to the sublicense terms in the accompanying license agreement).

    You should refer to the license agreement accompanying this Software for
    additional information regarding your rights and obligations.

    SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
    EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
    MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
    IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
    CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
    OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
    INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
    CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
    SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
    (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

*/


#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set MCP_RST aliases
#define MCP_RST_TRIS               TRISAbits.TRISA2
#define MCP_RST_LAT                LATAbits.LATA2
#define MCP_RST_PORT               PORTAbits.RA2
#define MCP_RST_WPU                WPUAbits.WPUA2
#define MCP_RST_OD                ODCONAbits.ODCA2
#define MCP_RST_ANS                ANSELAbits.ANSA2
#define MCP_RST_SetHigh()            do { LATAbits.LATA2 = 1; } while(0)
#define MCP_RST_SetLow()             do { LATAbits.LATA2 = 0; } while(0)
#define MCP_RST_Toggle()             do { LATAbits.LATA2 = ~LATAbits.LATA2; } while(0)
#define MCP_RST_GetValue()           PORTAbits.RA2
#define MCP_RST_SetDigitalInput()    do { TRISAbits.TRISA2 = 1; } while(0)
#define MCP_RST_SetDigitalOutput()   do { TRISAbits.TRISA2 = 0; } while(0)
#define MCP_RST_SetPullup()      do { WPUAbits.WPUA2 = 1; } while(0)
#define MCP_RST_ResetPullup()    do { WPUAbits.WPUA2 = 0; } while(0)
#define MCP_RST_SetPushPull()    do { ODCONAbits.ODCA2 = 1; } while(0)
#define MCP_RST_SetOpenDrain()   do { ODCONAbits.ODCA2 = 0; } while(0)
#define MCP_RST_SetAnalogMode()  do { ANSELAbits.ANSA2 = 1; } while(0)
#define MCP_RST_SetDigitalMode() do { ANSELAbits.ANSA2 = 0; } while(0)

// get/set LED_R aliases
#define LED_R_TRIS               TRISAbits.TRISA4
#define LED_R_LAT                LATAbits.LATA4
#define LED_R_PORT               PORTAbits.RA4
#define LED_R_WPU                WPUAbits.WPUA4
#define LED_R_OD                ODCONAbits.ODCA4
#define LED_R_ANS                ANSELAbits.ANSA4
#define LED_R_SetHigh()            do { LATAbits.LATA4 = 1; } while(0)
#define LED_R_SetLow()             do { LATAbits.LATA4 = 0; } while(0)
#define LED_R_Toggle()             do { LATAbits.LATA4 = ~LATAbits.LATA4; } while(0)
#define LED_R_GetValue()           PORTAbits.RA4
#define LED_R_SetDigitalInput()    do { TRISAbits.TRISA4 = 1; } while(0)
#define LED_R_SetDigitalOutput()   do { TRISAbits.TRISA4 = 0; } while(0)
#define LED_R_SetPullup()      do { WPUAbits.WPUA4 = 1; } while(0)
#define LED_R_ResetPullup()    do { WPUAbits.WPUA4 = 0; } while(0)
#define LED_R_SetPushPull()    do { ODCONAbits.ODCA4 = 1; } while(0)
#define LED_R_SetOpenDrain()   do { ODCONAbits.ODCA4 = 0; } while(0)
#define LED_R_SetAnalogMode()  do { ANSELAbits.ANSA4 = 1; } while(0)
#define LED_R_SetDigitalMode() do { ANSELAbits.ANSA4 = 0; } while(0)

// get/set LED_G aliases
#define LED_G_TRIS               TRISAbits.TRISA5
#define LED_G_LAT                LATAbits.LATA5
#define LED_G_PORT               PORTAbits.RA5
#define LED_G_WPU                WPUAbits.WPUA5
#define LED_G_OD                ODCONAbits.ODCA5
#define LED_G_ANS                ANSELAbits.ANSA5
#define LED_G_SetHigh()            do { LATAbits.LATA5 = 1; } while(0)
#define LED_G_SetLow()             do { LATAbits.LATA5 = 0; } while(0)
#define LED_G_Toggle()             do { LATAbits.LATA5 = ~LATAbits.LATA5; } while(0)
#define LED_G_GetValue()           PORTAbits.RA5
#define LED_G_SetDigitalInput()    do { TRISAbits.TRISA5 = 1; } while(0)
#define LED_G_SetDigitalOutput()   do { TRISAbits.TRISA5 = 0; } while(0)
#define LED_G_SetPullup()      do { WPUAbits.WPUA5 = 1; } while(0)
#define LED_G_ResetPullup()    do { WPUAbits.WPUA5 = 0; } while(0)
#define LED_G_SetPushPull()    do { ODCONAbits.ODCA5 = 1; } while(0)
#define LED_G_SetOpenDrain()   do { ODCONAbits.ODCA5 = 0; } while(0)
#define LED_G_SetAnalogMode()  do { ANSELAbits.ANSA5 = 1; } while(0)
#define LED_G_SetDigitalMode() do { ANSELAbits.ANSA5 = 0; } while(0)

// get/set RC0 procedures
#define RC0_SetHigh()    do { LATCbits.LATC0 = 1; } while(0)
#define RC0_SetLow()   do { LATCbits.LATC0 = 0; } while(0)
#define RC0_Toggle()   do { LATCbits.LATC0 = ~LATCbits.LATC0; } while(0)
#define RC0_GetValue()         PORTCbits.RC0
#define RC0_SetDigitalInput()   do { TRISCbits.TRISC0 = 1; } while(0)
#define RC0_SetDigitalOutput()  do { TRISCbits.TRISC0 = 0; } while(0)
#define RC0_SetPullup()     do { WPUCbits.WPUC0 = 1; } while(0)
#define RC0_ResetPullup()   do { WPUCbits.WPUC0 = 0; } while(0)
#define RC0_SetAnalogMode() do { ANSELCbits.ANSC0 = 1; } while(0)
#define RC0_SetDigitalMode()do { ANSELCbits.ANSC0 = 0; } while(0)

// get/set RC1 procedures
#define RC1_SetHigh()    do { LATCbits.LATC1 = 1; } while(0)
#define RC1_SetLow()   do { LATCbits.LATC1 = 0; } while(0)
#define RC1_Toggle()   do { LATCbits.LATC1 = ~LATCbits.LATC1; } while(0)
#define RC1_GetValue()         PORTCbits.RC1
#define RC1_SetDigitalInput()   do { TRISCbits.TRISC1 = 1; } while(0)
#define RC1_SetDigitalOutput()  do { TRISCbits.TRISC1 = 0; } while(0)
#define RC1_SetPullup()     do { WPUCbits.WPUC1 = 1; } while(0)
#define RC1_ResetPullup()   do { WPUCbits.WPUC1 = 0; } while(0)
#define RC1_SetAnalogMode() do { ANSELCbits.ANSC1 = 1; } while(0)
#define RC1_SetDigitalMode()do { ANSELCbits.ANSC1 = 0; } while(0)

// get/set MODE aliases
#define MODE_TRIS               TRISCbits.TRISC2
#define MODE_LAT                LATCbits.LATC2
#define MODE_PORT               PORTCbits.RC2
#define MODE_WPU                WPUCbits.WPUC2
#define MODE_OD                ODCONCbits.ODCC2
#define MODE_ANS                ANSELCbits.ANSC2
#define MODE_SetHigh()            do { LATCbits.LATC2 = 1; } while(0)
#define MODE_SetLow()             do { LATCbits.LATC2 = 0; } while(0)
#define MODE_Toggle()             do { LATCbits.LATC2 = ~LATCbits.LATC2; } while(0)
#define MODE_GetValue()           PORTCbits.RC2
#define MODE_SetDigitalInput()    do { TRISCbits.TRISC2 = 1; } while(0)
#define MODE_SetDigitalOutput()   do { TRISCbits.TRISC2 = 0; } while(0)
#define MODE_SetPullup()      do { WPUCbits.WPUC2 = 1; } while(0)
#define MODE_ResetPullup()    do { WPUCbits.WPUC2 = 0; } while(0)
#define MODE_SetPushPull()    do { ODCONCbits.ODCC2 = 1; } while(0)
#define MODE_SetOpenDrain()   do { ODCONCbits.ODCC2 = 0; } while(0)
#define MODE_SetAnalogMode()  do { ANSELCbits.ANSC2 = 1; } while(0)
#define MODE_SetDigitalMode() do { ANSELCbits.ANSC2 = 0; } while(0)

// get/set WDFEED2 aliases
#define WDFEED2_TRIS               TRISCbits.TRISC3
#define WDFEED2_LAT                LATCbits.LATC3
#define WDFEED2_PORT               PORTCbits.RC3
#define WDFEED2_WPU                WPUCbits.WPUC3
#define WDFEED2_OD                ODCONCbits.ODCC3
#define WDFEED2_ANS                ANSELCbits.ANSC3
#define WDFEED2_SetHigh()            do { LATCbits.LATC3 = 1; } while(0)
#define WDFEED2_SetLow()             do { LATCbits.LATC3 = 0; } while(0)
#define WDFEED2_Toggle()             do { LATCbits.LATC3 = ~LATCbits.LATC3; } while(0)
#define WDFEED2_GetValue()           PORTCbits.RC3
#define WDFEED2_SetDigitalInput()    do { TRISCbits.TRISC3 = 1; } while(0)
#define WDFEED2_SetDigitalOutput()   do { TRISCbits.TRISC3 = 0; } while(0)
#define WDFEED2_SetPullup()      do { WPUCbits.WPUC3 = 1; } while(0)
#define WDFEED2_ResetPullup()    do { WPUCbits.WPUC3 = 0; } while(0)
#define WDFEED2_SetPushPull()    do { ODCONCbits.ODCC3 = 1; } while(0)
#define WDFEED2_SetOpenDrain()   do { ODCONCbits.ODCC3 = 0; } while(0)
#define WDFEED2_SetAnalogMode()  do { ANSELCbits.ANSC3 = 1; } while(0)
#define WDFEED2_SetDigitalMode() do { ANSELCbits.ANSC3 = 0; } while(0)

/**
   @Param
    none
   @Returns
    none
   @Description
    GPIO and peripheral I/O initialization
   @Example
    PIN_MANAGER_Initialize();
 */
void PIN_MANAGER_Initialize (void);

/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handling routine
 * @Example
    PIN_MANAGER_IOC();
 */



#endif // PIN_MANAGER_H
/**
 End of File
*/