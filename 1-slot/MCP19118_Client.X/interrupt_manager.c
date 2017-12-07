/**
  Generated Interrupt Manager Source File

  @Company:
    Microchip Technology Inc.

  @File Name:
    interrupt_manager.c

  @Summary:
    This is the Interrupt Manager file generated using MPLAB(c) Code Configurator

  @Description:
    This header file provides implementations for global interrupt handling.
    For individual peripheral handlers please see the peripheral driver for
    all modules selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB(c) Code Configurator - 4.0
        Device            :  PIC16F18325
        Driver Version    :  1.02
    The generated drivers are tested against the following:
        Compiler          :  XC8 1.35
        MPLAB             :  MPLAB X 3.40
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

#include "interrupt_manager.h"
#include "misc.h"

void interrupt INTERRUPT_InterruptManager (void)
{
    // interrupt handler
    if(PEIE == 1 && SSPIE == 1 && SSPIF == 1)
    {
        I2C_ISR();
    }
    if(T0IE == 1 && T0IF == 1)
    {
        Timer0_ISR();
    }
    /*
    if(IOCE == 1 && IOCF == 1)
    {
        IOCF = 0;
        IOCE = 0;
        if(PORTGPA & GPIOA_MODE_PIN){
            SSPIE = 0;
            //WDFEED_SetHigh();
        }
        else{
            SSPIF = 0;
            SSPIE = 1;
            //WDFEED_SetLow();
        }
        IOCE = 1;
    }
    */
    //if(PEIE == 1 && ADIE == 1 && ADIF == 1)
    //{
    //    ADC_ISR();
    //}
    if(PEIE == 1 && VINIE == 1 && VINIF == 1)
    {
        VINIF = 0;
        Turn_Off_Output();
        mcp.u16ErrorStatus |= INUVP;
        mcp.u16MCPStatus |= MCP_ERROR;
    }
    if(PEIE == 1 && OCIE == 1 && OCIF == 1)
    {
        OCIF = 0;
        mcp.u16ErrorStatus |= OUTOCP;
        mcp.u16MCPStatus |= MCP_ERROR;
    }
    if(PEIE == 1 && OVIE == 1 && OVIF == 1)
    {
        OVIF = 0;
        mcp.u16ErrorStatus |= OUTOCP;
        mcp.u16MCPStatus |= MCP_ERROR;
    }
    else
    {
        //Unhandled Interrupt
    }
}
/**
 End of File
*/
