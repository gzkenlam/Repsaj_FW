//-----------------------------------------------------------------------------
// FILENAME:  eeprom.c
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares functions to read/write A2D gain correction factors from/to EEPROM of the MCU
//
// %IF Zebra_Internal
//
// NOTES:    
//
// AUTHOR:   		Wasath Mudalige
// CREATION DATE: 	03/25/2016
// DERIVED FROM: 	New File
//
// EDIT HISTORY:
//
//
//
// %End 
//-----------------------------------------------------------------------------

#include <avr/io.h>
#include "eeprom.h"
#include "a2d.h"
#include "sysparam.h"
#include "Debug.h"

//-----------------------------------------------------------------------------
//
// Prototype:	WriteEEPWord
//
// Description:	Routine to write one word in EEPROM along with checksum
//
// Parameters:	data : Parameter to be stored in the EEPROM
//				index: index of the parameter
//				0-3: Constants for battery voltage sense for slot 0 to 3 respectively
//				4-7: Constants for battery current sense for slot 0 to 3 respectively
//
// Returns:		None
//
// Notes:	Checksum is calculated such that unsigned addition of "data" & "checksum" is equal to zero
//-----------------------------------------------------------------------------
void WriteEEPData(uint16_t addr, uint8_t size, uint8_t* pdatabuf)
{
	uint8_t *pbuf;
	uint8_t cnt, start = 0, cksum = 0;
	uint16_t start_addr;
	
	// |<--------- Slot 0 -------->|<--------- Slot 1 -------->|<--------- Slot 2 -------->|<--------- Slot 3 -------->|
	// |------|------|-----|-------|------|------|-----|-------|------|------|-----|-------|------|------|-----|-------|
	// | LSB  | MSB  |Cksum|NotUsed| LSB  | MSB  |Cksum|NotUsed| LSB  | MSB  |Cksum|NotUsed| LSB  | MSB  |Cksum|NotUsed| Batt Voltage constants
	// |------|------|-----|-------|------|------|-----|-------|------|------|-----|-------|------|------|-----|-------|
	//	   0	  1		2		3	   4	  5		6		7	   8	 9		10	   11	  12	 13		14	   15   (EEPROM Address)

	// |<--------- Slot 0 -------->|<--------- Slot 1 -------->|<--------- Slot 2 -------->|<--------- Slot 3 -------->|
	// |------|------|-----|-------|------|------|-----|-------|------|------|-----|-------|------|------|-----|-------|
	// | LSB  | MSB  |Cksum|NotUsed| LSB  | MSB  |Cksum|NotUsed| LSB  | MSB  |Cksum|NotUsed| LSB  | MSB  |Cksum|NotUsed| Batt Current constants
	// |------|------|-----|-------|------|------|-----|-------|------|------|-----|-------|------|------|-----|-------|
	//	   16	 17		18		19	  20	 21		22		23	   24	 25		26	   27	  28	 29		30	   31   (EEPROM Address)
	
	do
	{
		while (NVM.STATUS & NVM_NVMBUSY_bm);	// Wait if NVM is busy

		// Step1: Erase EEPROM Page Buffer
		NVM.CMD = NVM_CMD_ERASE_EEPROM_BUFFER_gc;
		CPU_CCP = CCP_IOREG_gc;
		NVM.CTRLA = NVM_CMDEX_bm;
		while (NVM.STATUS & NVM_NVMBUSY_bm);	// Wait if NVM is busy

		// Step2: Write data to Page Buffer
		start_addr = addr & 0x3E0;
		pbuf = (void*)(MAPPED_EEPROM_START + addr);
		
		for (cnt=start; cnt<size; cnt++)
		{
			*pbuf = *pdatabuf;
			cksum += *pdatabuf;
			pdatabuf++;
			pbuf++;
			addr++;
			if ((addr % EEPROM_PAGE_SIZE) == 0)
			{
				start = cnt+1;
				break;
			}
		}
		
		if (size == cnt)
			*pbuf = 0xFF-cksum;// Set the checksum

		// Step3: Perform Erase & Write EEPROM Page in one operation
		NVM.CMD = NVM_CMD_ERASE_WRITE_EEPROM_PAGE_gc;	// Erase and write in atomic operation
		NVM.ADDR0 = start_addr & 0xff;
		NVM.ADDR1 = (uint8_t)(start_addr >> 8);
		CPU_CCP = CCP_IOREG_gc;
		NVM.CTRLA = NVM_CMDEX_bm;				// Execute NVM command

		NVM.CMD = NVM_CMD_NO_OPERATION_gc;
	}
	while (cnt < size);
}

//-----------------------------------------------------------------------------
//
// Prototype:	ReadEEPWord
//
// Description:	Routine to read one word from EEPROM
//
// Parameters:	index : index of the parameter to be retrieved
//				0-3: Constants for battery voltage sense for slot 0 to 3 respectively
//				4-7: Constants for battery current sense for slot 0 to 3 respectively
//
// Returns:		If the checksum is valid, return the value stored in EEPROM
//				Otherwise returns the default value
//
// Notes:
//-----------------------------------------------------------------------------
uint16_t ReadEEPWord(uint8_t index)
{
	uint8_t data_lo, data_hi, ck_sum;
	uint8_t *pbuf;

	while (NVM.STATUS & NVM_NVMBUSY_bm);	// Wait if NVM is busy

	pbuf = (void*)(MAPPED_EEPROM_START + (4*index));
	data_lo = *pbuf;						// Read LSB
	data_hi = *(pbuf+1);					// Read MSB
	ck_sum = (uint8_t)(data_lo + data_hi + *(pbuf+2));	// Calculate checksum
		
	if (ck_sum == 0xFF)						// If checksum is correct
		return ((uint16_t)(data_hi<<8) | data_lo);	// return the value stored in EEPROM

	if (index < INDEX_I_SENSE_CONST)		// If reading voltage constants
		return DEF_V_SENSE_CONST;			// Checksum is invalid, so return the default value

	return DEF_I_SENSE_CONST;
}
