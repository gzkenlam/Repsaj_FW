/**
  Section: Included Files
*/

#include <xc.h>
#include "memory.h"
#include"../GenericTypeDefs.h"
/**
  Section: Flash Module APIs
*/

void  ReadWordsFlash(uint32_t sourceAddr,uint16_t word_length, uint8_t * buffer)
{
    uint16_t i;
    WORD_VAL temp_data;
    uint8_t GIEBitValue = INTCONbits.GIE;   // Save interrupt enable
    
    for(i=0;i<word_length; i++)
    {    
        INTCONbits.GIE = 0;     // Disable interrupts
        NVMADR = sourceAddr;
        NVMCON1bits.NVMREGS = 0;    // Deselect Configuration space
        NVMCON1bits.RD = 1;      // Initiate Read
        NOP();
        NOP();
        INTCONbits.GIE = GIEBitValue;	// Restore interrupt enable
        temp_data.Val = NVMDAT;
        *buffer++= temp_data.byte.LB;
        *buffer++ = temp_data.byte.HB;
        sourceAddr ++;
    }
}

void EraseBlockFlash(uint32_t startAddr)
{
    uint8_t GIEBitValue = INTCONbits.GIE;   // Save interrupt enable
    INTCONbits.GIE = 0; // Disable interrupts
    // Load lower 8 bits of erase address boundary
    NVMADRL = (startAddr & 0xFF);
    // Load upper 6 bits of erase address boundary
    NVMADRH = ((startAddr & 0xFF00) >> 8);

    // Block erase sequence
    NVMCON1bits.NVMREGS = 0;    // Deselect Configuration space
    NVMCON1bits.FREE = 1;    // Specify an erase operation
    NVMCON1bits.WREN = 1;    // Allows erase cycles

    // Start of required sequence to initiate erase
    NVMCON2 = 0x55;
    NVMCON2 = 0xAA;
    NVMCON1bits.WR = 1;      // Set WR bit to begin erase
    NOP();
    NOP();

    NVMCON1bits.WREN = 0;       // Disable writes
    INTCONbits.GIE = GIEBitValue;	// Restore interrupt enable
}

void WriteBlockFlash(uint32_t writeAddr, uint8_t *flash_array)
{
    uint16_t    blockStartAddr  = (uint16_t )(writeAddr & ((END_FLASH-1) ^ (ERASE_FLASH_BLOCKSIZE-1)));
    uint8_t     GIEBitValue = INTCONbits.GIE;   // Save interrupt enable
    uint8_t i;
    // Flash write must start at the beginning of a row
    if( writeAddr != blockStartAddr )
    {
        return ;
    }

    INTCONbits.GIE = 0;         // Disable interrupts
    
    

    // Block write sequence
    NVMCON1bits.NVMREGS = 0;    // Deselect Configuration space
    NVMCON1bits.WREN = 1;    // Enable wrties
    NVMCON1bits.LWLO = 1;    // Only load write latches

    for (i=0; i<WRITE_FLASH_BLOCKSIZE; i++)
    {
        // Load lower 8 bits of write address
        NVMADRL = (writeAddr & 0xFF);
        // Load upper 6 bits of write address
        NVMADRH = ((writeAddr & 0xFF00) >> 8);

	// Load data in current address
        NVMDATL = *flash_array++;
        NVMDATH = *flash_array++;

        if(i == (WRITE_FLASH_BLOCKSIZE-1))
        {
            // Start Flash program memory write
            NVMCON1bits.LWLO = 0;
        }

        NVMCON2 = 0x55;
        NVMCON2 = 0xAA;
        NVMCON1bits.WR = 1;
        NOP();
        NOP();

	writeAddr++;
    }

    NVMCON1bits.WREN = 0;       // Disable writes
    INTCONbits.GIE = GIEBitValue;   // Restore interrupt enable

    return ;
}



/**
 End of File
*/
