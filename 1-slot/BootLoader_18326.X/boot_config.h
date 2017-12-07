/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include"GenericTypeDefs.h"
// TODO Insert appropriate #include <>
/** D E F I N I T I O N S ****************************************************/

    #define WRITE_FLASH_BLOCKSIZE_BYTE 	(64ul)
    #define ERASE_FLASH_BLOCKSIZE_BYTE 	(64ul)
    #define END_BLOACK_FLASH_ADDR    (END_FLASH-1) 

//*****************************************************************************
#define RM_RESET_VECTOR			0x0600
#define RM_INTERRUPT_VECTOR    0x0604
#define MAX_PACKET_SIZE				256	//Max packet size
	

/****************************************************************************
*****************************************************************************/

#define BOOTBYTESL					0x00
#define BOOTBYTESH					0x06  // Bootloader size  0x00 - 0x7FF
#define VERSIONL					0x01
#define VERSIONH					0x05
#define COMMANDMASKH				0x01
#define	COMMANDMASKL				0x82  // [COMMANDMASKL:FAMILYID]not used in PIC18  ; familyid  4= PIC18; 2 = PIC16
#define	STARTBOOTL					0x00		
#define	STARTBOOTH					0x00		
#define	STARTBOOTU					0x00	
#define DEVICEIDL                   0xA4   // 18326 device id = 0x30A4 18325 device id = 0x303E; 18345 id = 0x303F
#define DEVICEIDH                   0x30
#define COMMAND_NACK 		0xAA
#define COMMAND_ACK   		0x55

#define BootloaderInfo_CMD		0x00 
#define ReadBytesFlash_CMD		0x01
#define VerifyFlash_CMD     	0x02
#define EraseBlockFlash_CMD		0x03
#define WriteFlash_CMD			0x04
#define ReadEeprom_CMD          0x05
#define WriteEeprom_CMD    		0x06
#define WriteConfig_CMD   		0x07
#define GotoAppVector_CMD   	0x08





//Communications Control bytes
#define STX 0x0F   //?????????
#define ETX 0x04   // ?????????
#define DLE 0x05  // ??????????

extern void ResetDevice();
extern uint16_t crc16_ccitt2(uint8_t data, uint16_t crc);
extern uint16_t crc16_ccitt1(uint8_t *,uint16_t);
void SendEscapeByte(uint8_t );
extern void ResetDevice();
extern void PutChar(uint8_t);
extern void GetChar(uint8_t *);
extern void GetCommand();
extern void HandleCommand();
//extern void PutResponse(uint16_t);
//extern void Err_Display();
extern void ReadWordsFlash(uint32_t , uint16_t , uint8_t *);
extern void EraseBlockFlash(uint32_t addr);
extern void WriteBlockFlash(uint32_t ,  uint8_t *);
#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

