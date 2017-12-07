/**
  EUSART Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    eusart.c

  @Summary
    This is the generated driver implementation file for the EUSART driver using MPLAB(c) Code Configurator

  @Description
    This header file provides implementations for driver APIs for EUSART.
    Generation Information :
        Product Revision  :  MPLAB(c) Code Configurator - 4.15
        Device            :  PIC16F18325
        Driver Version    :  2.00
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

/**
  Section: Included Files
*/
#include "eusart.h"
#include    "pin_manager.h"
#include "memory.h"
#include "../boot_config.h"
extern uint16_t u16DelayCount;
extern uint8_t u8Delay;
extern  uint8_t u8BootMode;

//void ResetDevice(void);
/**
  Section: EUSART APIs
*/

void EUSART_Initialize(void)
{
    // Set the EUSART module to the options selected in the user interface.
    LATC &= ~0x03;
    TRISC &= ~ 0x02;
    WPUC &= ~0x03;
    
    bool state = GIE;
    GIE = 0;
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x00; // unlock PPS

    RC1PPSbits.RC1PPS = 0x14;   //RC1->EUSART:TX;
    RXPPSbits.RXPPS = 0x10;   //RC0->EUSART:RX;

    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0x01; // lock PPS

    GIE = state;

    // ABDOVF no_overflow; SCKP Non-Inverted; BRG16 16bit_generator; WUE disabled; ABDEN disabled; 
    BAUD1CON = 0x08;

    // SPEN enabled; RX9 8-bit; CREN enabled; ADDEN disabled; SREN disabled; 
    RC1STA = 0x90;

    // TX9 8-bit; TX9D 0; SENDB sync_break_complete; TXEN enabled; SYNC asynchronous; BRGH hi_speed; CSRC slave; 
    TX1STA = 0x24;

    // Baud Rate = 19200; SP1BRGL 160; 
    SP1BRGL = 0xA0;

    // Baud Rate = 19200; SP1BRGH 1; 
    SP1BRGH = 0x01;

}

void PutChar(uint8_t txChar)
{

	TXREG1 = txChar;	//put character onto UART FIFO to transmit
	while(!TXSTA1bits.TRMT);	//wait for transmit to finish

}//end PutChar(BYTE Char)
void GetChar(uint8_t * ptrChar)
{
//	uint8_t dummy;
    static uint16_t u16BlinkCount;
	while(1)
	{	
		//check for receive errors
        CLRWDT();
		if(RCSTA1bits.OERR)
		{
			RCSTA1bits.CREN = 0;
			RCSTA1bits.CREN = 1;
		}
		//get the data
		if(PIR1bits.RCIF)
		{
			*ptrChar = RCREG1;
            //if(u8BootMode < 5){
            //    u8BootMode ++;
            //    if(u8BootMode >4){
            //    LED_G_SetLow();
            //    }
            //}
			break;
		}
        if(u16BlinkCount++ > 60000){
            u16BlinkCount = 0;
            LED_G_Toggle();
        }
        /*
        if(u8BootMode <5){
            if(u8Delay > 0)
            {
                if(u16DelayCount > 0){
                    u16DelayCount--;
                }
                else{
                    u8Delay--;
                    u16DelayCount = 60000;
                }
            }
            else
            {
                LED_G_SetLow();
                ReadWordsFlash(RM_RESET_VECTOR,1, &temp_data.byte.LB);
                if((temp_data.byte.LB !=0xff) ||(temp_data.byte.HB!=0x3f)) //Check if the MCU  operate Bootloader before( address =0x1010)
                {
                    ResetDevice();
                }
            }
        }*/
	}//end while(1)
}//end GetChar(BYTE *ptrChar)

/**
  End of File
*/
