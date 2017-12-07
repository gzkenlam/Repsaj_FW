//-----------------------------------------------------------------------------
// FILENAME:  battcomm.h
//
// Copyright(c) 2016 Zebra Technologies Inc. All rights reserved.
//
// DESCRIPTION: battery communication function prototypes are defined in this file
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
#ifndef _BATTCOMM_H_
#define _BATTCOMM_H_

#define ENCRYPT_DATA	1
#define DECRYPT_DATA	0

// Battery communication defs
#define ACK_bm				0x80
#define NACK_bm				0x40

typedef enum
{
	DATA_WRITE = 0,
	DATA_READ
} ACCESS_TYPE;

typedef enum
{
// ------ Note: Don't change the order ------
// ------- Firmware Update I2C States -------
	READ_ID_FROM_FUB = 0,	// 00
	VERIFY_DEV_ID_FW_VER,	// 01
	
// ------ FUB Programming I2C States --------
	WRITE_FUB_PAGE,			// 02
	WAIT_FUB_PAGE_WRITE,	// 03
	DO_NEXT_FUB_PAGE_WRITE,	// 04
	
// --- Dead battery detection starts here ---
	CHECK_DEAD_BATT_CHRG ,	// 05
	CHECK_DEAD_BATT_CHRG2,	// 06
	CHECK_DEAD_BATT_CHRG3,	// 07
	DONE_DEAD_BATT_CHRG,	// 08
	
// ------ QLN Battery Found States -------
	READ_DEV_NAME_QLN,		// 09
	VERIFY_DEV_NAME_QLN,	// 0A
	READ_CHRG_CURR_QLN,		// 0B
	READ_DESIGN_CAP_QLN,	// 0C
	
	READ_DATE_MFG_QLN,		// 0D
	READ_BATT_SN_QLN,		// 0E
	READ_CC_THRESHOLD_QLN,	// 0F

	SET_BATT_PRESENT_FLAG,	// 10
	
//------- ZQ3 battery authentication states ---------
	AUTH_INIT,				// 11
	AUTH_PUT_SLEEP_ZQ3,		// 12
	AUTH_PUT_SLEEP_RLX,		// 13
	AUTH_WAKE,				// 14
	AUTH_READ_SLOT_DATA,	// 15
	AUTH_COPY_READ_DATA,	// 16
	
	AUTH_GEN_RANDON_NUMBER,	// 17
	AUTH_COPY_RND_NUM,		// 18
	AUTH_GEN_DIGEST,		// 19
	AUTH_READ_ENCRYPTED,	// 1A
	AUTH_COPY_READ_ENCRYPTED,// 1B
	
	AUTH_NONCE_MLB,			// 1C
	AUTH_DIGEST_RND_NUM,	// 1D
	AUTH_DERIVE_KEY,		// 1E
	AUTH_READ_SLOT4_MLB,	// 1F
	
	AUTH_SETUP_SLOT_DATA,	// 20
	AUTH_WRITE_SLOT_DATA,	// 21
	AUTH_CHECK_WRITE_STATUS,// 22
	
	AUTH_SHA256_START,		// 23
	AUTH_SHA256_PUBLIC,		// 24
	AUTH_SHA256_UPDATE,		// 25
	AUTH_SHA256_END,		// 26
	AUTH_VERIFY_EXTERN,		// 27
	AUTH_CHECK_STATUS,		// 28
	
	AUTH_READ_CHRG_DATA,	// 29
	AUTH_VERIFY_CHRG_DATA,	// 2A

// ------ Dead battery charging starts here -------
	SET_A2D_MUXES_DEAD,		// 2B
	READ_A2D_CURR_TMP_DEAD,	// 2C
	JUST_SLOW_CHRG_DEAD,	// 2D

// ------- QLN Battery I2C States --------
	READ_BATT_VOLT_QLN,		// 2E
	READ_AVG_CURR_QLN,		// 2F
	READ_INST_CURR_QLN,		// 30
	READ_BATT_TEMP_QLN,		// 31
	READ_BATT_STATUS_QLN,	// 32
	READ_CYCLE_CNT_QLN,		// 33
	READ_SOC_QLN,			// 34
	READ_SOH_QLN,			// 35
	WAIT_A2D_SETUP_DELAY_QLN,// 36
	READ_A2D_CURR_TMP_QLN,	// 37
	CHARGE_ENTRY_QLN,		// 38
	
// ------- ZQ3 Battery I2C States --------
	READ_BATT_VOLT_ZQ3,		// 39
	READ_AVG_CURR_ZQ3,		// 3A
	READ_INST_CURR_ZQ3,		// 3B
	READ_BATT_TEMP_ZQ3,		// 3C
	READ_BATT_STATUS_ZQ3,	// 3D
	READ_CYCLE_CNT_ZQ3,		// 3E
	READ_SOC_ZQ3,			// 3F
	READ_SOH_ZQ3,			// 40
	WAIT_A2D_SETUP_DELAY_ZQ3,// 41
	READ_A2D_CURR_TMP_ZQ3,	// 42
	CHARGE_ENTRY_ZQ3,		// 43

	BATT_REMOVED,			// 44
	BATT_REMOVED_VIRTUAL,	// 45
	
// ------- Battery/Charger Fault States ----
	BATT_FAULT,		// 46
	CHRG_FAULT,		// 47
	COMM_FAULT,		// 48
	SIMPLY_WAIT		// 49
} BATT_I2C_STATES;

// Encryption routine start address in BootLoader area
#define AESEncryptDecrypt32K 		0x4758
#define AESEncryptDecrypt16K		0x2758

// Function prototypes
void Do_I2C(void);
void AccessBattChip(uint8_t slv_addr, uint8_t pointer, uint8_t read_cnt, uint8_t write_cnt, uint8_t *pbuf);
void UpdateBattCommStatus(uint8_t state);
void EncryptDecryptBattData(uint8_t type);
void DoDumbCalc(void);

#endif	// BattComm.h
