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

// get/set IO_RA0 aliases
#define IO_RA0_TRIS               TRISAbits.TRISA0
#define IO_RA0_LAT                LATAbits.LATA0
#define IO_RA0_PORT               PORTAbits.RA0
#define IO_RA0_WPU                WPUAbits.WPUA0
#define IO_RA0_OD                ODCONAbits.ODCA0
#define IO_RA0_ANS                ANSELAbits.ANSA0
#define IO_RA0_SetHigh()            do { LATAbits.LATA0 = 1; } while(0)
#define IO_RA0_SetLow()             do { LATAbits.LATA0 = 0; } while(0)
#define IO_RA0_Toggle()             do { LATAbits.LATA0 = ~LATAbits.LATA0; } while(0)
#define IO_RA0_GetValue()           PORTAbits.RA0
#define IO_RA0_SetDigitalInput()    do { TRISAbits.TRISA0 = 1; } while(0)
#define IO_RA0_SetDigitalOutput()   do { TRISAbits.TRISA0 = 0; } while(0)
#define IO_RA0_SetPullup()      do { WPUAbits.WPUA0 = 1; } while(0)
#define IO_RA0_ResetPullup()    do { WPUAbits.WPUA0 = 0; } while(0)
#define IO_RA0_SetPushPull()    do { ODCONAbits.ODCA0 = 1; } while(0)
#define IO_RA0_SetOpenDrain()   do { ODCONAbits.ODCA0 = 0; } while(0)
#define IO_RA0_SetAnalogMode()  do { ANSELAbits.ANSA0 = 1; } while(0)
#define IO_RA0_SetDigitalMode() do { ANSELAbits.ANSA0 = 0; } while(0)

// get/set EX_REST aliases
#define EX_REST_TRIS               TRISAbits.TRISA2
#define EX_REST_LAT                LATAbits.LATA2
#define EX_REST_PORT               PORTAbits.RA2
#define EX_REST_WPU                WPUAbits.WPUA2
#define EX_REST_OD                ODCONAbits.ODCA2
#define EX_REST_ANS                ANSELAbits.ANSA2
#define EX_REST_SetHigh()            do { LATAbits.LATA2 = 1; } while(0)
#define EX_REST_SetLow()             do { LATAbits.LATA2 = 0; } while(0)
#define EX_REST_Toggle()             do { LATAbits.LATA2 = ~LATAbits.LATA2; } while(0)
#define EX_REST_GetValue()           PORTAbits.RA2
#define EX_REST_SetDigitalInput()    do { TRISAbits.TRISA2 = 1; } while(0)
#define EX_REST_SetDigitalOutput()   do { TRISAbits.TRISA2 = 0; } while(0)
#define EX_REST_SetPullup()      do { WPUAbits.WPUA2 = 1; } while(0)
#define EX_REST_ResetPullup()    do { WPUAbits.WPUA2 = 0; } while(0)
#define EX_REST_SetPushPull()    do { ODCONAbits.ODCA2 = 1; } while(0)
#define EX_REST_SetOpenDrain()   do { ODCONAbits.ODCA2 = 0; } while(0)
#define EX_REST_SetAnalogMode()  do { ANSELAbits.ANSA2 = 1; } while(0)
#define EX_REST_SetDigitalMode() do { ANSELAbits.ANSA2 = 0; } while(0)

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

// get/set BT_MODE aliases
#define BT_MODE_TRIS               TRISCbits.TRISC2
#define BT_MODE_LAT                LATCbits.LATC2
#define BT_MODE_PORT               PORTCbits.RC2
#define BT_MODE_WPU                WPUCbits.WPUC2
#define BT_MODE_OD                ODCONCbits.ODCC2
#define BT_MODE_ANS                ANSELCbits.ANSC2
#define BT_MODE_SetHigh()            do { LATCbits.LATC2 = 1; } while(0)
#define BT_MODE_SetLow()             do { LATCbits.LATC2 = 0; } while(0)
#define BT_MODE_Toggle()             do { LATCbits.LATC2 = ~LATCbits.LATC2; } while(0)
#define BT_MODE_GetValue()           PORTCbits.RC2
#define BT_MODE_SetDigitalInput()    do { TRISCbits.TRISC2 = 1; } while(0)
#define BT_MODE_SetDigitalOutput()   do { TRISCbits.TRISC2 = 0; } while(0)
#define BT_MODE_SetPullup()      do { WPUCbits.WPUC2 = 1; } while(0)
#define BT_MODE_ResetPullup()    do { WPUCbits.WPUC2 = 0; } while(0)
#define BT_MODE_SetPushPull()    do { ODCONCbits.ODCC2 = 1; } while(0)
#define BT_MODE_SetOpenDrain()   do { ODCONCbits.ODCC2 = 0; } while(0)
#define BT_MODE_SetAnalogMode()  do { ANSELCbits.ANSC2 = 1; } while(0)
#define BT_MODE_SetDigitalMode() do { ANSELCbits.ANSC2 = 0; } while(0)

// get/set TEST aliases
#define TEST_TRIS               TRISCbits.TRISC3
#define TEST_LAT                LATCbits.LATC3
#define TEST_PORT               PORTCbits.RC3
#define TEST_WPU                WPUCbits.WPUC3
#define TEST_OD                ODCONCbits.ODCC3
#define TEST_ANS                ANSELCbits.ANSC3
#define TEST_SetHigh()            do { LATCbits.LATC3 = 1; } while(0)
#define TEST_SetLow()             do { LATCbits.LATC3 = 0; } while(0)
#define TEST_Toggle()             do { LATCbits.LATC3 = ~LATCbits.LATC3; } while(0)
#define TEST_GetValue()           PORTCbits.RC3
#define TEST_SetDigitalInput()    do { TRISCbits.TRISC3 = 1; } while(0)
#define TEST_SetDigitalOutput()   do { TRISCbits.TRISC3 = 0; } while(0)
#define TEST_SetPullup()      do { WPUCbits.WPUC3 = 1; } while(0)
#define TEST_ResetPullup()    do { WPUCbits.WPUC3 = 0; } while(0)
#define TEST_SetPushPull()    do { ODCONCbits.ODCC3 = 1; } while(0)
#define TEST_SetOpenDrain()   do { ODCONCbits.ODCC3 = 0; } while(0)
#define TEST_SetAnalogMode()  do { ANSELCbits.ANSC3 = 1; } while(0)
#define TEST_SetDigitalMode() do { ANSELCbits.ANSC3 = 0; } while(0)

// get/set TXD aliases
#define TXD_TRIS               TRISCbits.TRISC4
#define TXD_LAT                LATCbits.LATC4
#define TXD_PORT               PORTCbits.RC4
#define TXD_WPU                WPUCbits.WPUC4
#define TXD_OD                ODCONCbits.ODCC4
#define TXD_ANS                ANSELCbits.ANSC4
#define TXD_SetHigh()            do { LATCbits.LATC4 = 1; } while(0)
#define TXD_SetLow()             do { LATCbits.LATC4 = 0; } while(0)
#define TXD_Toggle()             do { LATCbits.LATC4 = ~LATCbits.LATC4; } while(0)
#define TXD_GetValue()           PORTCbits.RC4
#define TXD_SetDigitalInput()    do { TRISCbits.TRISC4 = 1; } while(0)
#define TXD_SetDigitalOutput()   do { TRISCbits.TRISC4 = 0; } while(0)
#define TXD_SetPullup()      do { WPUCbits.WPUC4 = 1; } while(0)
#define TXD_ResetPullup()    do { WPUCbits.WPUC4 = 0; } while(0)
#define TXD_SetPushPull()    do { ODCONCbits.ODCC4 = 1; } while(0)
#define TXD_SetOpenDrain()   do { ODCONCbits.ODCC4 = 0; } while(0)
#define TXD_SetAnalogMode()  do { ANSELCbits.ANSC4 = 1; } while(0)
#define TXD_SetDigitalMode() do { ANSELCbits.ANSC4 = 0; } while(0)

// get/set RXD aliases
#define RXD_TRIS               TRISCbits.TRISC5
#define RXD_LAT                LATCbits.LATC5
#define RXD_PORT               PORTCbits.RC5
#define RXD_WPU                WPUCbits.WPUC5
#define RXD_OD                ODCONCbits.ODCC5
#define RXD_ANS                ANSELCbits.ANSC5
#define RXD_SetHigh()            do { LATCbits.LATC5 = 1; } while(0)
#define RXD_SetLow()             do { LATCbits.LATC5 = 0; } while(0)
#define RXD_Toggle()             do { LATCbits.LATC5 = ~LATCbits.LATC5; } while(0)
#define RXD_GetValue()           PORTCbits.RC5
#define RXD_SetDigitalInput()    do { TRISCbits.TRISC5 = 1; } while(0)
#define RXD_SetDigitalOutput()   do { TRISCbits.TRISC5 = 0; } while(0)
#define RXD_SetPullup()      do { WPUCbits.WPUC5 = 1; } while(0)
#define RXD_ResetPullup()    do { WPUCbits.WPUC5 = 0; } while(0)
#define RXD_SetPushPull()    do { ODCONCbits.ODCC5 = 1; } while(0)
#define RXD_SetOpenDrain()   do { ODCONCbits.ODCC5 = 0; } while(0)
#define RXD_SetAnalogMode()  do { ANSELCbits.ANSC5 = 1; } while(0)
#define RXD_SetDigitalMode() do { ANSELCbits.ANSC5 = 0; } while(0)

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
void PIN_MANAGER_IOC(void);



#endif // PIN_MANAGER_H
/**
 End of File
*/