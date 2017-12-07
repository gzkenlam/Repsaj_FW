//------------------------------------------------------------------------------------
// FILENAME:  cyrpto.S
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: Declares cryptographic functions for FUB data encryption and decryption
//
// %IF Zebra_Internal
//
// NOTES: 	Written in pure assembly
//
// AUTHOR:   		Wasath Mudalige
// CREATION DATE: 	05/03/2016
// DERIVED FROM: 	New File
//
// EDIT HISTORY:
//
//
//
// %End 
//------------------------------------------------------------------------------------
#define _SFR_ASM_COMPAT		1
#define _SFR_OFFSET			0
#include <avr/io.h>

#define EKEY_SIZE		0x10	// Size of the encryption keys
#define BLOCK_SIZE		0x10	// Size of the data block that can be encrypted/ decrypted at a time
#define EEP_READ_bp		0
#define LOCK_BLOCK_ADDR	0x30
#define ADDR_P0_bp		1
#define ADDR_P0_bm		(1<<ADDR_P0_bp)

//------------------------------------------------------------------------------------
//
// Prototype: 	AES_EncryptDecrept
//
// Description:	Routine to encrypt/ decrypt FUB data
//
// Parameters:	R16: Action to be performd (0:Decrypt, 1:Encrypt: 2:Decrypt with last byte)
//				X Reg: RAM address of "EA.DataBuffer"
//				Y Reg: Address of the EEPROM (YL:0x00~0xFF, YH=0x00 or 0x02)
//
// Returns:		None
// 
// Notes:	This function is located in fixed addr of boot loader flash
//			The toaster firmware calls this function using icall instruction. This enables
//			it to return back to toaster code once the encryption/ decryption is done
//------------------------------------------------------------------------------------

.section .AES_CRYPTO, "ax"	; a-section is allocatable, x-section is executable
AES_EncryptDecrypt:
	;Disable interrupts and backup the registers we are going to use in Crypto routine
	push	R18
	in	R18, SREG	; Read SREG
	push	R18		; Backup SREG
	clr	R18
	out	SREG, R18	; Disable global interrupts

	push	R19
	push	R26
	push	R27
	push	R28
	push	R29
    push	R30
    push	R31

StartCrypto:
	lds	R19, PR_PRGEN			; Enable clocks to AES Crypto engine
	andi	R19, ~PR_AES_bm
	sts	PR_PRGEN, R19

	ldi R19, AES_RESET_bm
	sts	AES_CTRL, R19			; Reset AES Crypto engine

ResetWait:
	lds	R19, AES_CTRL
	sbrc	R19, AES_RESET_bp
	rjmp	ResetWait			; Wait till reset completes
	
	;Clear AES Status register
	ldi	R19, (AES_ERROR_bm | AES_SRIF_bm)
	sts	AES_STATUS, R19

	; Encryption key is 16 bytes long. We load key in a loop one byte at time
	ldi	R19, EKEY_SIZE			; Set the byte counter with size of the encryption key
    ldi	R31, hi8(EKEY1)			; Set the address of 1st encryption key
	ldi	R30, lo8(EKEY1)

	;Check weather we are accessing the "lock block"
CheckLockBlock:
	cpi	R28, LOCK_BLOCK_ADDR	; If battery data address equals to lock block address
	brne	LoadAESKey			; Nop
	sbrc	R29, ADDR_P0_bp		; If accessing first half (0 to 255 bytes) of the eeprom
	rjmp	LoadAESKey			; Nop
	ldi	R31, hi8(EKEY2)			; Set the address of 2nd encryption key to as we are accessing the "lock block"
	ldi	R30, lo8(EKEY2)

	; Loop to load the encryption key
LoadAESKey:	
	lpm	R18, Z+					; Load the key (one byte at a time) from program memory
	sts	AES_KEY, R18			; Write the key to KEY register
	dec	R19						; Decrement the byte counter
	brne	LoadAESKey			; Loop until all 16 bytes get loaded

	sbrc	R16, EEP_READ_bp	; Is this a data decryption
	rjmp	DoDataEncrypt		; Nop, continue with encryption
	
	; It is required to obtain the last sub key for data decryption. In order to obtain last sub key,
	; Xmega data sheet recommends encrypting a dummy data blcok. 
	ldi	R19, BLOCK_SIZE			; Set the byte counter with size of data block
	clr	R18						; Zero is choosen as dummy data
	; Loop to load dummy data to encryption engine
LoadDummyData:
	sts	AES_STATE, R18			; Load dummy data to State register
	dec	R19						; Decrement the byte counter
	brne	LoadDummyData		; Loop until all 16 bytes get loaded

	ldi	R19, AES_START_bm		
	sts	AES_CTRL, R19			; Start the encryption (Don't need to read and modify the AES CTRL reg as non of the other bits were set before)

BusyWait:
	lds	R19, AES_STATUS
	sbrs	R19, AES_SRIF_bp
	rjmp	BusyWait			; Wait till encyption of dummy data completes

	;Clear AES Status register
	ldi	R19, (AES_ERROR_bm | AES_SRIF_bm)
	sts	AES_STATUS, R19

	ldi	R19, AES_DECRYPT_bm		; Set direction as decrypt
	sts	AES_CTRL, R19			; Note: Don't need to read and modify AES CTRL reg as non of the other bits were set before

	; Its time to load the actual data to be encrypted/ decrypted
DoDataEncrypt:
	movw	R30, R26
	ldi	R19, (BLOCK_SIZE-1)		; Load only the first 15 bytes of the data in a loop

	; Loop to load data to be encrypted/ decrypted
LoadAESData:
	ld	R18, Z+					; Read the data from RAM (1 byte at a time)
	sts	AES_STATE, R18			; Load the STATE register
	eor	R28, R18				; xor with data to prepare the random number for last data byte
	dec R19						; Decrement the byte counter
	brne	LoadAESData			; Loop until all 15 bytes get loaded

	lds	R18, RTC_CNT			; Xor with RTC CNT to make the random number TURE random
	eor	R18, R28
	sbrs	R16, EEP_READ_bp	; If this a data decreption
	ld	R18, Z					; Yes, load the last data byte from RAM
	sts	AES_STATE, R18			; Load the last data byte to STATE register
	
	; We have set the direction bit, load the key and data
	; Now its time to start the encryption. Must read and modify the AES CTRL reg just to preserve the direction bit
	lds	R19, AES_CTRL
	ori	R19, AES_START_bm
	sts	AES_CTRL, R19			; Start the encryption engine 

	; Loop to wait until encryption/ decryption completes
WaitTillComplete:
	lds	R19, AES_STATUS
	sbrs	R19, AES_SRIF_bp
	rjmp	WaitTillComplete

	; Data encryption/ decryption completes, Now its time to copy the result back into RAM
	ldi	R19, (BLOCK_SIZE-1)		; Copy only the first 15 bytes of result in a loop
	clr	R28
	; Loop to copy result into RAM
CopyData:
	sts	AES_KEY, R28			; Clear KEY register for security reasons
	lds	R18, AES_STATE			; Read the result (1 byte at a time)
	st	X+, R18					; Copy it into RAM and incremenet to next RAM location
	dec	R19						; Decrement the byte counter
	brne	CopyData			; Loop until all 15 bytes get copied

	sts	AES_KEY, R28			; Clear KEY register for security reasons
	lds	R18, AES_STATE			; Make sure we read out last (16th) byte from STATE reg
	tst 16						; if this is a data encryption or Toaster firmware decryption
	brne	CopyLastByte		; Yes, copy last byte
	clr	R18						; Make the last byte 0
CopyLastByte:
	st	X+, R18					; Copy the last byte into RAM

ExitCrypto:
	lds	R19, PR_PRGEN			; Stop clocks to AES module (saves 140uA)
	ori	R19, PR_AES_bm
	sts	PR_PRGEN, R19
	 	
	; Restore the registers we have backed up
    pop	R31
	pop	R30
	pop R29
	pop R28
	pop	R27
	pop	R26
	pop	R19
	pop	R18 
	out	SREG, R18			; Restore SREG
	pop R18
	 
	ret    

// Place the encryption keys at the bottom of the boot loader area, This approach enables us to PATCH the keys
// Encryption Key1 is located at 0x8FE0 - 0x8FEF 
// Encryption Key2 is located at 0x8FF0 - 0x8FFF
.section .EN_KEYS, "ax", @progbits
.type EKEY1, @object
.size EKEY1, EKEY_SIZE
EKEY1:	; "0123456789ABCDEF"	 Encryption key1
	.byte 0x9E, 0xCD, 0x9F, 0x18, 0x3B, 0xCB, 0x5A, 0x3F, 0x79, 0x31, 0x6B, 0x34, 0x8A, 0x2C, 0xDB, 0x68
.type EKEY2, @object
.size EKEY2, EKEY_SIZE 
EKEY2:	; "FEDCBA9876543210" 	Encryption key2
	.byte 0x63, 0xC3, 0x80, 0x7E, 0xB2, 0xA6, 0xFA, 0xA0, 0xD5, 0x6E, 0x22, 0x7A, 0x04, 0xFC, 0x5E, 0xCE
