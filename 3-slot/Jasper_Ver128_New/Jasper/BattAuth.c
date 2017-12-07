/*
 * BattAuth.c
 *
 * Created: 6/2/2016 10:06:08 AM
 *  Author: FJB678
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "BattAuth.h"
#include "BattComm.h"
#include "I2C_Batt.h"
#include "Macros.h"
#include "Batt_Defs.h"
#include "Uart.h"

AUTH_DATA	AUTH;

void AuthVerifyExtern(uint8_t keyid, uint8_t *data)
{
	AUTH.ExecTime = 72;
	CommBuf[0] = ATCA_CMD_SIZE_MIN + SIG_LEN;
	CommBuf[1] = ATCA_VERIFY;
	CommBuf[2] = VERIFY_MODE_VALIDATEEXTERNAL;
	CommBuf[3] = keyid;
	CommBuf[4] = 0x00;
	memcpy(&CommBuf[5], data, SIG_LEN);

	atCRC(&CommBuf[5 + SIG_LEN]);
	AccessBattChip(AUTH_SLV_ADDR_MLB, 0x03, 0, (ATCA_CMD_SIZE_MIN + SIG_LEN), CommBuf);
}

void AuthNouce(uint8_t i2c_slv_addr, uint8_t mode)
{
	AUTH.ExecTime = 29;
	CommBuf[0] = ATCA_CMD_SIZE_MIN + RANDOM_NUM_SIZE;
	CommBuf[1] = ATCA_NONCE;
	CommBuf[2] = mode;
	CommBuf[3] = 0x00;
	CommBuf[4] = 0x00;
	memcpy(&CommBuf[5],  AUTH.RandomNum, RANDOM_NUM_SIZE);

	atCRC(&CommBuf[5 + RANDOM_NUM_SIZE]);
	AccessBattChip(i2c_slv_addr, 0x03, 0, (ATCA_CMD_SIZE_MIN + RANDOM_NUM_SIZE), CommBuf);
}

void AuthDigestRndNum(void)
{
	AUTH.ExecTime = 29;
	CommBuf[0] = ATCA_CMD_SIZE_MIN + 20;
	CommBuf[1] = ATCA_NONCE;
	CommBuf[2] = NONCE_MODE_SEED_UPDATE;
	CommBuf[3] = 0x00;
	CommBuf[4] = 0x80;
	memset(&CommBuf[5], 0, 20);

	atCRC(&CommBuf[25]);
	AccessBattChip(AUTH_SLV_ADDR_MLB, 0x03, 0, (ATCA_CMD_SIZE_MIN + 20), CommBuf);
}

void AuthDeriveKey(void)
{
	AUTH.ExecTime = 50;
	CommBuf[0] = ATCA_CMD_SIZE_MIN;
	CommBuf[1] = ATCA_DERIVE_KEY;
	CommBuf[2] = DERIVE_KEY_RANDOM_FLAG;
	CommBuf[3] = KEY_ID4;
	CommBuf[4] = 0x00;
	
	atCRC(&CommBuf[5]);
	AccessBattChip(AUTH_SLV_ADDR_MLB, 0x03, 0, ATCA_CMD_SIZE_MIN , CommBuf);
}

void AuthSHA256(uint8_t mode, uint8_t len, uint8_t *data)
{
	AUTH.ExecTime = 9;
	CommBuf[0] = ATCA_CMD_SIZE_MIN + len;
	CommBuf[1] = ATCA_SHA;
	CommBuf[2] = mode;
	CommBuf[3] = len;
	CommBuf[4] = 0x00;
	if (len)
		memcpy(&CommBuf[5], data, len);
		
	atCRC(&CommBuf[5+len]);
	AccessBattChip(AUTH_SLV_ADDR_MLB, 0x03, 0, (ATCA_CMD_SIZE_MIN + len), CommBuf);
}

void AuthGenDig(void)
{
	AUTH.ExecTime = 11;
	CommBuf[0] = ATCA_CMD_SIZE_MIN + ATCA_SHA_DIGEST_SIZE;
	CommBuf[1] = ATCA_GENDIG;
	CommBuf[2] = ATCA_ZONE_DATA;
	CommBuf[3] = KEY_ID2;
	memset(&CommBuf[4], 0, ATCA_SHA_DIGEST_SIZE + 1);
	CommBuf[5] = 0x1C;
	CommBuf[6] = 0x04;
	CommBuf[7] = 0x04;
	atCRC(&CommBuf[5+ATCA_SHA_DIGEST_SIZE]);
	AccessBattChip(AUTH.BattSlvAddr, 0x03, 0, (ATCA_CMD_SIZE_MIN + ATCA_SHA_DIGEST_SIZE), CommBuf);
}

void AuthReadData(void)
{
	uint8_t zone;
	if (AUTH.SlotAddrLo)
		zone = ATCA_ZONE_DATA;
	else
		zone = ATCA_ZONE_CONFIG;
		
	AUTH.ExecTime = 2;
	CommBuf[0] = ATCA_CMD_SIZE_MIN;
	CommBuf[1] = ATCA_READ;
	CommBuf[2] = (ATCA_ZONE_READWRITE_32 | zone);
	if (AUTH.I2CSlvAddr == AUTH_SLV_ADDR_MLB)
	{
		CommBuf[3] = ADDR_SLOT4;
		CommBuf[4] = ADDR_BLK0;
	}
	else
	{
		CommBuf[3] = AUTH.SlotAddrLo;
		CommBuf[4] = AUTH.SlotAddrHi;
	}
	atCRC(&CommBuf[5]);
	AccessBattChip(AUTH.I2CSlvAddr, 0x03, 0, ATCA_CMD_SIZE_MIN, CommBuf);
}

void AuthWriteData(void)
{
	AUTH.ExecTime = 28;
	CommBuf[0] = ATCA_CMD_SIZE_MIN + ATCA_BLOCK_SIZE;
	CommBuf[1] = ATCA_WRITE;
	CommBuf[2] = (ATCA_ZONE_READWRITE_32 | ATCA_ZONE_DATA);
	CommBuf[3] = AUTH.SlotAddrLo;
	CommBuf[4] = AUTH.SlotAddrHi;
	memcpy(&CommBuf[5], AUTH.RandomNum, RANDOM_NUM_SIZE);
	atCRC(&CommBuf[5 + RANDOM_NUM_SIZE]);
	AccessBattChip(AUTH_SLV_ADDR_MLB, 0x03, 0, (ATCA_CMD_SIZE_MIN + RANDOM_NUM_SIZE), CommBuf);
}

void CheckAuthResponse(void)
{
	uint8_t crc[ATCA_CRC_SIZE], length;
	atCRC(crc);
	
	length = CommBuf[0]-ATCA_CRC_SIZE;
	if ((crc[0] == CommBuf[length]) && (crc[1] == CommBuf[length+1]))	// If CRC checksum is correct
	{
		length--;
		if (length <= MAX_RESP_SIZE)					// If buffer is large enough to hold the response
		{
			AUTH.RespLen = length;						// Set the response length
			memcpy(AUTH.RespBuf, &CommBuf[1], length);	// Copy the response
		}
	}
}

void CopyResp(uint8_t *read_buf, uint8_t offset, uint8_t len)
{
	memcpy(read_buf, &AUTH.RespBuf[offset], len);
}

void WaitAuthExecTime(uint8_t wait_time)
{
	pBatt->I2CBusy = TRUE;				// Set I2C busy flag
	TCC1_CNT = 0;
	TCC1_CCA = M_SECS(wait_time);
	TCC1_INTFLAGS = (TC1_CCAIF_bm | TC1_CCBIF_bm | TC1_OVFIF_bm);
	TCC1_INTCTRLB = TC_CCAINTLVL_MED_gc;
	TCC1_CTRLA = TC_CLKSEL_DIV64_gc;
	SLEEP_CTRL = SLEEP_SEN_bm;				// Enable Sleep at idle mode
	SYS.Actives |= AUTH_TMR_bm;
}

SIGNAL(TCC1_CCA_vect)
{
	TCC1_CTRLA = TC_CLKSEL_OFF_gc;
	TCC1_INTCTRLB = TC_CCAINTLVL_OFF_gc;
	SYS.Actives &= ~AUTH_TMR_bm;
	Read_I2CAuth(0xFF);	
}

void atCRC(uint8_t *crc_buf)
{
	uint8_t counter;
	uint16_t crc_register = 0;
	uint16_t polynom = 0x8005;
	uint8_t shift_register;
	uint8_t data_bit, crc_bit;
	uint8_t length = CommBuf[0] - ATCA_CRC_SIZE;

	for (counter = 0; counter < length; counter++)
	{
		for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1)
		{
			data_bit = (CommBuf[counter] & shift_register) ? 1 : 0;
			crc_bit = crc_register >> 15;
			crc_register <<= 1;
			if (data_bit != crc_bit)
				crc_register ^= polynom;
		}
	}
	*crc_buf = (uint8_t)(crc_register & 0x00FF);
	crc_buf++;
	*crc_buf = (uint8_t)(crc_register >> 8);
	AUTH.RespLen = 0;
}
