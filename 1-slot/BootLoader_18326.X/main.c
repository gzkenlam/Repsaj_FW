/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using MPLAB(c) Code Configurator

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB(c) Code Configurator - 4.15
        Device            :  PIC16F18345
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

#include "mcc_generated_files/mcc.h"
#include "boot_config.h"
#define _str(x)  #x
#define str(x)  _str(x)

volatile uint16_t		Receive_Checksum;
volatile uint32_t       BootStartAddr,BootEndAddr;
volatile uint8_t        Flash_Erase_Flag=0;
uint8_t  buffer[MAX_PACKET_SIZE];
uint16_t u16DelayCount;
uint8_t u8Delay;
//uint8_t u8BootMode = 0;
/*
                         Main application
 */
void ResetDevice()
{
     asm ("pagesel " str(RM_RESET_VECTOR));
    asm ("goto  "  str(RM_RESET_VECTOR));   
   
}
/*********************************************************************
; Function: 	void ResetDevice();
;
; PreCondition: None.
;
; Input:    	none
;                               
; Output:   	None.
;
; Side Effects: None.
;
; Overview: 	used to vector to user code
;**********************************************************************/


void interrupt server_isr (void)
{
    asm ("pagesel " str(RM_INTERRUPT_VECTOR));
    asm ("goto  "  str(RM_INTERRUPT_VECTOR));   
  
}

void BootMode(void)
{
    if((PORTCbits.RC0 == 0) && (PORTCbits.RC1 == 0))
    {
        LED_R_SetHigh();
        while((PORTCbits.RC0 == 0) && (PORTCbits.RC1 == 0)){
            if(u16DelayCount <60000){
                u16DelayCount ++;
                CLRWDT();
            }
            else{
                CLRWDT();
                if(u8Delay < 15){
                    u8Delay ++;
                    u16DelayCount = 0;
                }
                else{
                    LED_R_SetLow();
                    u8Delay++;
                    u16DelayCount = 0;
                    if(u8Delay > 30)
                    {
                        LED_R_SetHigh();
                        return;
                    }
                }
            }
        }
        if(u8Delay < 15){
            return;
        }
        MCP_RST_SetLow();  
        EUSART_Initialize();
        while(1){
            GetCommand();		//Get full  command from UART
            LED_R_SetHigh();
            HandleCommand();	//Handle the command
            LED_R_SetLow();
        }
        //break;
    }
    return;
}

/************************************************************************************
**
 * **
 * ************************************************************************************/

void main(void)
{
    //WORD_VAL temp_data;
    WORD_VAL temp_data;
	Receive_Checksum=0;	
    // initialize the device
    SYSTEM_Initialize();
    //LED_G_SetHigh();
    //LED_R_SetHigh();
    BootMode();
    
    ReadWordsFlash(RM_RESET_VECTOR,1, &temp_data.byte.LB);
	if((temp_data.byte.LB !=0xff) ||(temp_data.byte.HB!=0x3f)) //Check if the MCU  operate Bootloader before( address =0x1010)
	{
 		ResetDevice();
	}
    else{
        LED_R_SetHigh();
    }
/*
    if(0)//KEY_GetValue()!=0)                  // MCU will ,enter User application when BOOTLOADER_KEY not  pressed
	{
        ReadWordsFlash(RM_RESET_VECTOR,1, &temp_data.byte.LB);
		if((temp_data.byte.LB !=0xff) ||(temp_data.byte.HB!=0x3f)) //Check if the MCU  operate Bootloader before( address =0x1010)
		{
 			ResetDevice();
		}		
	}
  */
    
}



/********************************************************************
* Function: 	       void GetCommand()
* Precondition:      UART Setup
* Input: 		       None.
* Output:		None.
* Side Effects:	None.
* Overview: 	       Polls the UART to recieve a complete AN851 command.
*			 	Fills buffer[1024] with recieved data.
* Note:		 	AN1310A.
********************************************************************/
//Frame Format

//[<STX>?]<STX>[<DATA>?]<CRCL><CRCH><ETX>[<ETX>]
void GetCommand()
{
	BYTE RXByte;
	WORD NewChecksum,ReceiveChecksum;
	WORD dataCount;
	dataCount = 0;
	while(dataCount <= MAX_PACKET_SIZE)//maximum num bytes
	{	
		GetChar(&RXByte);		
		switch(RXByte)
		{   
			case STX: 			//Start over if STX
				PutChar(STX);  		// Re-send Start byte to Host PC
				break;
			case ETX:			//End of packet if ETX
				ReceiveChecksum = buffer[dataCount-2]+( (WORD)buffer[dataCount-1]<<8);
				NewChecksum = crc16_ccitt1(buffer,dataCount-2);
				if(ReceiveChecksum == NewChecksum)
				return;	//return if OK
				dataCount = 0;
				PutChar(STX);  		// Re-send Start byte to Host PC
				break;
			case DLE:			//If DLE, treat next as data
				GetChar(&RXByte);
			default:			//get data, put in buffer
				buffer[dataCount++] = RXByte;
				break;
		}//end switch(RXByte)
	}//end while(dataCount <= MAX_PACKET_SIZE+1)
}//end GetCommand()
/********************************************************************
* Function: 	     void HandleCommand()
* Precondition:    data in buffer
* Input: 		     None.
* Output:		     None.
* Side Effects:    None.
* Overview: 	     Handles commands received from host
* Note:		     None.
********************************************************************/
void HandleCommand()
{
	uint8_t Command;
    uint16_t i,j,temp_length,Erase_length,Write_length,Verify_length,Read_length;

	WORD_VAL checksum;
	DWORD_VAL sourceAddr;
	WORD_VAL  Verify_CRC;
	Command 		= buffer[0];	   //get command from buffer
	sourceAddr.v[0] = buffer[1];		
	sourceAddr.v[1] = buffer[2];
	sourceAddr.v[2] = buffer[3];
	sourceAddr.v[3] = 0;

	//Handle Commands		
	switch(Command)
	{

/******************************************************************************************************
*ReadBootloader Info: 
*requester: <STX> <0x00> <CRCL><CRCH> <ETX>
*response : <STX> <BOOTBYTESL><BOOTBYTESH> <VERSIONL><VERSIONH> <COMMANDMASKH> <COMMANDMASKL:FAMILYID>
*                <STARTBOOTL><STARTBOOTH><STARTBOOTU><0x00><DEVICEIDL><DEVICEIDL> <CRCL><CRCH> <ETX>
******************************************************************************************************/
	
		case BootloaderInfo_CMD:						//Read version	
			buffer[0] = BOOTBYTESL;
			buffer[1] = BOOTBYTESH;
			buffer[2] = VERSIONL;
			buffer[3] = VERSIONH;
			
			buffer[4] = COMMANDMASKH;
			buffer[5] = COMMANDMASKL;
			
			buffer[6] = STARTBOOTL;
			buffer[7] = STARTBOOTH;
			buffer[8] = STARTBOOTU;
			buffer[9] = 0x00; //COMMAND
            buffer[10]= DEVICEIDL;
            buffer[11]= DEVICEIDH;
			BootStartAddr = buffer[6]+(uint16_t)buffer[7]<<8+(DWORD)buffer[8]<<16;
			BootEndAddr   = BootStartAddr+buffer[0]+(WORD)buffer[1]<<8;
			checksum.Val = crc16_ccitt1(buffer,12);
			buffer[12]= checksum.byte.LB;
			buffer[13]= checksum.byte.HB;
			//Output buffer as response packet
			for(i = 0; i < 14; i++)
			{
				SendEscapeByte(buffer[i]);
			}
			PutChar(ETX);
            NOP();
			break;
/******************************************************************************************************
*ReadBytesFlash: 
*requester: <STX> <0x01> <ADDRESSL><ADDRESSH><ADDRESU><0x00> <BYTESL><BYTESH> <CRCH><CRCL> <ETX>
*response : <STX> [DATA?] <CRCL><CRCH> <ETX>
*******************************************************************************************************/
		
		case ReadBytesFlash_CMD:						//Read flash memory
			//get  address from buffer
            Verify_CRC.Val = 0;
			Read_length = (uint8_t)buffer[5]+((uint8_t)buffer[6]<<8);
			PutChar(STX);
 
            while(Read_length)  // note: please treat the Read_length as the word length, as the GUI bug for PIC16
            {
                if(Read_length >= sizeof(buffer)/2)
                    temp_length = sizeof(buffer)/2;
                else 
                    temp_length = Read_length;
                
                ReadWordsFlash(sourceAddr.Val,temp_length, buffer);
                Read_length -= temp_length;
                sourceAddr.Val+=temp_length;
                for(i=0;i<temp_length*2;i++)
                {
                    SendEscapeByte(buffer[i]);
                    Verify_CRC.Val= crc16_ccitt2(buffer[i], Verify_CRC.Val);
                }
            }
            SendEscapeByte(Verify_CRC.byte.LB);
            SendEscapeByte(Verify_CRC.byte.HB);
			PutChar(ETX);
			
			break;
/******************************************************************************************************
*verifyFlash: 
*requester: <STX> <0x02> <ADDRESSL><ADDRESSH><ADDRESSU><0x00> <BLOCKSL><BLOCKSH> <CRCL><CRCH> <ETX>
*response : <STX>[<CRC1L><CRC1H>?<CRCnL><CRCnH>]<ETX>
*******************************************************************************************************/
		case VerifyFlash_CMD:
			//get  address from buffer
			Verify_CRC.Val = 0;
			Verify_length   = buffer[5];
			PutChar(STX);

			for(i=0;i<(Verify_length);i++)
			{
                ReadWordsFlash(sourceAddr.Val,ERASE_FLASH_BLOCKSIZE, buffer);
                for(j=0;j<ERASE_FLASH_BLOCKSIZE_BYTE;j++)
                {
                    Verify_CRC.Val= crc16_ccitt2(buffer[j], Verify_CRC.Val);
                }
            	SendEscapeByte(Verify_CRC.byte.LB);
            	SendEscapeByte(Verify_CRC.byte.HB);
				sourceAddr.Val +=ERASE_FLASH_BLOCKSIZE; 
			}
			PutChar(ETX);
			break;
/******************************************************************************************************
*WriteFlash: 
*requester: <STX> <0x04> <ADDRESSL><ADDRESSH><ADDRESSU><0x00> <BLOCKSL> [<DATA>?] <CRCL><CRCH> <ETX>
*response : <STX> <0x04> <CRCL><CRCH> <ETX>
*******************************************************************************************************/
			
		case WriteFlash_CMD:						//Write flash memory
			Write_length   = buffer[5];//*FLASH_WRITE_BLOCK;			//how much section Write block size received?
			for(i=0;i<Write_length;i++)
			{
				if(((sourceAddr.Val<BootStartAddr)||(sourceAddr.Val>=BootEndAddr))&&(sourceAddr.Val!=END_BLOACK_FLASH_ADDR))
				{
                    // Block erase sequence
                    EraseBlockFlash(sourceAddr.Val);
                    j = 0x1000;
                    while(--j);
                    WriteBlockFlash(sourceAddr.Val, &buffer[6+WRITE_FLASH_BLOCKSIZE_BYTE*i]);
				}
				sourceAddr.Val = sourceAddr.Val+WRITE_FLASH_BLOCKSIZE;
			}
			Write_length =0;
			while(--Write_length);
    		PutChar(STX);

			buffer[0]= 0x04;
			buffer[1]= 0x84; // CRCL 
			buffer[2]= 0x40; //CRCH
			for(i = 0; i < 3; i++)
			{
				SendEscapeByte(buffer[i]);
			}
			PutChar(ETX);			
 			break;
		
/******************************************************************************************************
*EraseBlockFlash: 
*requester: <STX> <0x03> <ADDRESSL><ADDRESSH><ADDRESSU><0x00> <PAGESL> <CRCL><CRCH> <ETX>
*response : <STX> <0x03> <CRCL><CRCH> <ETX>
*******************************************************************************************************/
		case EraseBlockFlash_CMD:						//Erase flash memory
			Erase_length   = buffer[5];			//how much section Erase block size received?
			for(i=0;i<Erase_length;i++)
			{
				if(((sourceAddr.Val<BootStartAddr)||(sourceAddr.Val>=BootEndAddr))&&(sourceAddr.Val!=END_BLOACK_FLASH_ADDR))
				{

                    EraseBlockFlash(sourceAddr.Val);
                    
				}
				sourceAddr.Val -= ERASE_FLASH_BLOCKSIZE;
					
			}	
			PutChar(STX);
			buffer[0]= 0x03;
			buffer[1]= 0x63; // CRCL 
			buffer[2]= 0x30; //CRCH
			for(i = 0; i < 3; i++)
			{
				SendEscapeByte(buffer[i]);
			}
			PutChar(ETX);
			break;
/*******************************************************************************************************
*  <STX> <0X08> <CRCL><CRCH> <ETX>
********************************************************************************************************/
		case GotoAppVector_CMD:
			RCSTA1bits.SPEN = 0;  //Disable UART
			ResetDevice();
			
			break;
		case WriteConfig_CMD:
			break;
		default:
			break;
	}// end switch(Command)
}//end HandleCommand()

/*********************************************************
** CCITT CRC16 source code example  ???= (X16+X12+X5+1)
** ptr ??????len ????? 
**********************************************************/

uint16_t crc16_ccitt1(uint8_t *ptr,uint16_t len) 
{   
 	uint16_t crc;
 	uint8_t i;
 	crc	=	0;   
 	while(len--)   
 	{ 
		crc ^=	((WORD)*ptr<<8);  // crc +(data<<8)
		for(i=0;i<8;i++)
 		{                
			if((crc&0x8000)!=0) 
			{
 				crc<<=1; 
				crc^=0x1021; 
 			}              //1-1 
			else
			{
				crc<<=1;         //1-2
			} 

		}             
		ptr++;   
	}   
	return(crc); 
}
/*********************************************************
** CCITT CRC16 source code example ???= (X16+X12+X5+1)
** data ????crc ??????CRC?
** ?????:( data+crc )?CRC?
**********************************************************/
uint16_t crc16_ccitt2(uint8_t data, uint16_t crc)

 {   
 	uint8_t i;   
	crc ^= (uint16_t)data<<8;  // crc +(data<<8)
	for(i=0;i<8;i++) 
 	{                
		if((crc&0x8000)!=0) 
		{
 			crc<<=1; 
			crc^=0x1021; 
 		}              //1-1 
		else
		{
			crc<<=1;         //1-2
		} 
	}   
	return(crc); 
}
/******************************************************************************
** Write a byte to the serial port while escaping control characters with a DLE first.
**************************************************************************************/
void SendEscapeByte(BYTE TxData)
{
	if((TxData ==STX)||(TxData==ETX)||(TxData==DLE))
	{
		PutChar(DLE);
	}

	PutChar(TxData);
}
/**
 End of File
*/