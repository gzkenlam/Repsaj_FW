//-----------------------------------------------------------------------------
// FILENAME:  pwm.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: PWM data structures and function prototypes are defined in this file
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
#ifndef _PWM_H_
#define _PWM_H_

#define PWMIReg(slot)			(&TCC0_CCABUF + slot)
#define GetPWMI(slot)		 	(*PWMIReg(slot))

#define PWMVReg(slot)			(&TCE0_CCABUF + slot)
#define GetPWMV(slot)		 	(*PWMVReg(slot))

// ------ 12-bits PWM Voltage Table -----------
#define PWMV_LOOKUP_TABLE "\x06\x03\x1D\x03\x33\x03\x4A\x03\x60\x03\x77\x03\x8E\x03\xA5\x03\xBC\x03\xD3\x03\xEA\x03\x01\x04\x18\x04\x30\x04\x47\x04\x5E\x04\x76\x04\x8D\x04\xA5\x04\xBD\x04\xD4\x04\xEC\x04\x04\x05\x1C\x05\x34\x05\x4C\x05\x64\x05\x7C\x05\x94\x05\xAD\x05\xC5\x05\xDD\x05\xF6\x05\x0E\x06\x27\x06\x40\x06\x59\x06\x71\x06\x8A\x06\xA3\x06\xBC\x06\xD5\x06\xEF\x06\x08\x07\x21\x07\x3B\x07\x54\x07\x6D\x07\x87\x07\xA1\x07\xBA\x07\xD4\x07\xEE\x07\x08\x08\x22\x08\x3C\x08\x56\x08\x71\x08\x8B\x08\xA5\x08\xC0\x08\xDB\x08\xF5\x08\x10\x09\x2B\x09\x45\x09\x60\x09\x7B\x09\x97\x09\xB2\x09\xCD\x09\xE8\x09\x04\x0A\x1F\x0A\x3B\x0A\x56\x0A\x72\x0A\x8E\x0A\xAA\x0A\xC6\x0A\xE2\x0A"


// Function Prototypes
void Init_PWM(void);
void Deinit_PWM(void);
void SetPWMV(uint8_t slot, uint16_t value);
void SetPWMI(uint8_t slot, uint16_t value);
void SetClrPWMActives(uint8_t bitpos, uint16_t value);
void SetActives(uint8_t active);

#endif /* a2d.h */
