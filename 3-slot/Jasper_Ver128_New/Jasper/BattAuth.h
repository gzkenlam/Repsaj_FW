/*
 * BattAuth.h
 *
 * Created: 6/2/2016 10:01:14 AM
 *  Author: FJB678
 */ 


#ifndef BATTAUTH_H_
#define BATTAUTH_H_

// Auth defines
#define AUTH_SLV_ADDR_ZQ3	0xC2	// ZQ3 battery auth chip slave address
#define AUTH_SLV_ADDR_RLX	0xC4	// Rolex battery auth chip slave address
#define AUTH_SLV_ADDR_MLB	0xC0	// MLB auth chip slave address
#define MAX_AUTH_CMD_SIZE	135

#define MAX_RESP_SIZE	64
#define KEY_LEN			64
#define SIG_LEN			64
#define EXTRA_LEN		64
#define ATCA_SN_SIZE	9                                 //!< number of bytes in the device serial number
#define M_SECS(n)		((uint16_t)188*n)
#define DEVICE_PRIV		0
#define RANDOM_NUM_SIZE                 ((uint8_t)0x20)     //!< Number of bytes in the data packet of a random command

typedef struct  
{
	uint8_t ExecTime;
	uint8_t RespLen;
	
	uint8_t RespBuf[MAX_RESP_SIZE];
	
	uint8_t ClientPubKey[KEY_LEN];	// Client public key
	uint8_t ClientSignat[SIG_LEN];	// Client signature
	
	uint8_t SubconPubKey[KEY_LEN];	// Sub con public key
	uint8_t SubconSignat[SIG_LEN];	// Sub con signature
	
	uint8_t ClientExtra[EXTRA_LEN];	// Client extra data
	uint8_t SubconExtra[EXTRA_LEN];	// Sub con extra data
	
	uint8_t RandomNum[RANDOM_NUM_SIZE];
	
	uint8_t SlotAddrHi;
	uint8_t SlotAddrLo;
	uint8_t I2CSlvAddr;
	uint8_t BattSlvAddr;
	
} AUTH_DATA;

#define AUTH_SN0		0x01
#define AUTH_SN1		0x23
#define AUTH_SN8		0xDE

#define ADDR_BLK0		0x00
#define ADDR_BLK1		0x01
#define ADDR_BLK2		0x02
#define ADDR_BLK4		0x04
#define ADDR_BLK5		0x05

#define ADDR_SLOT0		0x00
#define ADDR_SLOT4		0x20
#define ADDR_SLOT7		0x38
#define ADDR_SLOT8		0x40
#define ADDR_SLOT9		0x48
#define ADDR_SLOT10		0x50
#define ADDR_SLOT11		0x58
#define ADDR_SLOT12		0x60
#define ADDR_SLOT13		0x68
#define ADDR_SLOT14		0x70

#define GENKEY_MODE_PUBLIC  0x00         //!< GenKey mode: public key calculation
#define ATCA_SHA_DIGEST_SIZE        (32)
#define ATCA_BLOCK_SIZE             (32)                                //!< size of a block
#define ATCA_COUNT_SIZE             ((uint8_t)1)                        //!< Number of bytes in the command packet Count
#define ATCA_CRC_SIZE               ((uint8_t)2)                        //!< Number of bytes in the command packet CRC
#define ATCA_PACKET_OVERHEAD        (ATCA_COUNT_SIZE + ATCA_CRC_SIZE)   //!< Number of bytes in the command packet

#define AUTH_WAKEUP_DELAY	1	// 1 mSec
#define AUTH_SLEEP_DELAY	1	// 1 mSec
#define AUTH_SLEEP_REG		0x01
#define AUTH_IDLE_REG		0x02
#define AUTH_CMD_REG		0x03

#define KEY_ID2				2
#define KEY_ID4				4
#define KEY_ID10			10
#define KEY_ID12			12


//! minimum number of bytes in command (from count byte to second CRC byte)
#define ATCA_CMD_SIZE_MIN       ((uint8_t)7)
//! maximum size of command packet (Verify)
#define ATCA_CMD_SIZE_MAX       ((uint8_t)4 * 36 + 7)
//! status byte for success
#define CMD_STATUS_SUCCESS      ((uint8_t)0x00)
//! status byte after wake-up
#define CMD_STATUS_WAKEUP       ((uint8_t)0x11)
//! command parse error
#define CMD_STATUS_BYTE_PARSE   ((uint8_t)0x03)
//! command ECC error
#define CMD_STATUS_BYTE_ECC     ((uint8_t)0x05)
//! command execution error
#define CMD_STATUS_BYTE_EXEC    ((uint8_t)0x0F)
//! communication error
#define CMD_STATUS_BYTE_COMM    ((uint8_t)0xFF)

// Opcodes for ATATECC Commands
#define ATCA_CHECKMAC                   ((uint8_t)0x28)         //!< CheckMac command op-code
#define ATCA_DERIVE_KEY                 ((uint8_t)0x1C)         //!< DeriveKey command op-code
#define ATCA_INFO                       ((uint8_t)0x30)         //!< Info command op-code
#define ATCA_GENDIG                     ((uint8_t)0x15)         //!< GenDig command op-code
#define ATCA_GENKEY                     ((uint8_t)0x40)         //!< GenKey command op-code
#define ATCA_HMAC                       ((uint8_t)0x11)         //!< HMAC command op-code
#define ATCA_LOCK                       ((uint8_t)0x17)         //!< Lock command op-code
#define ATCA_MAC                        ((uint8_t)0x08)         //!< MAC command op-code
#define ATCA_NONCE                      ((uint8_t)0x16)         //!< Nonce command op-code
#define ATCA_PAUSE                      ((uint8_t)0x01)         //!< Pause command op-code
#define ATCA_PRIVWRITE                  ((uint8_t)0x46)         //!< PrivWrite command op-code
#define ATCA_RANDOM                     ((uint8_t)0x1B)         //!< Random command op-code
#define ATCA_READ                       ((uint8_t)0x02)         //!< Read command op-code
#define ATCA_SIGN                       ((uint8_t)0x41)         //!< Sign command op-code
#define ATCA_UPDATE_EXTRA               ((uint8_t)0x20)         //!< UpdateExtra command op-code
#define ATCA_VERIFY                     ((uint8_t)0x45)         //!< GenKey command op-code
#define ATCA_WRITE                      ((uint8_t)0x12)         //!< Write command op-code
#define ATCA_ECDH                       ((uint8_t)0x43)         //!< ECDH command op-code
#define ATCA_COUNTER                    ((uint8_t)0x24)         //!< Counter command op-code
#define ATCA_SHA                        ((uint8_t)0x47)         //!< SHA command op-code

/** \name Definitions for Zone and Address Parameters
   @{ */
#define ATCA_ZONE_CONFIG                ((uint8_t)0x00)         //!< Configuration zone
#define ATCA_ZONE_OTP                   ((uint8_t)0x01)         //!< OTP (One Time Programming) zone
#define ATCA_ZONE_DATA                  ((uint8_t)0x02)         //!< Data zone
#define ATCA_ZONE_MASK                  ((uint8_t)0x03)         //!< Zone mask
#define ATCA_ZONE_ENCRYPTED             ((uint8_t)0x40)         //!< Zone bit 6 set: Write is encrypted with an unlocked data zone.
#define ATCA_ZONE_READWRITE_32          ((uint8_t)0x80)         //!< Zone bit 7 set: Access 32 bytes, otherwise 4 bytes.
#define ATCA_ADDRESS_MASK_CONFIG        (0x001F)                //!< Address bits 5 to 7 are 0 for Configuration zone.
#define ATCA_ADDRESS_MASK_OTP           (0x000F)                //!< Address bits 4 to 7 are 0 for OTP zone.
#define ATCA_ADDRESS_MASK               (0x007F)                //!< Address bit 7 to 15 are always 0.
/** @} */

#define SHA_MODE_SHA256_START			((uint8_t)0x00)		//!< Initialization, does not accept a message
#define SHA_MODE_SHA256_UPDATE          ((uint8_t)0x01)		//!< Add 64 bytes in the meesage to the SHA context
#define SHA_MODE_SHA256_END             ((uint8_t)0x02)		//!< Complete the calculation and return the digest
#define SHA_SHA256_PUBLIC_MASK          ((uint8_t)0x03)		//!< Add 64 bytes in the slot to the SHA context

#define NONCE_MODE_SEED_UPDATE          ((uint8_t)0x00)     //!< Nonce mode: update seed
#define NONCE_MODE_NO_SEED_UPDATE       ((uint8_t)0x01)     //!< Nonce mode: do not update seed
#define NONCE_MODE_INVALID              ((uint8_t)0x02)     //!< Nonce mode 2 is invalid.
#define NONCE_MODE_PASSTHROUGH          ((uint8_t)0x03)     //!< Nonce mode: pass-through

#define VERIFY_MODE_VALIDATEEXTERNAL	((uint8_t)0x01)     //!< Verify mode: validate external
#define VERIFY_MODE_EXTERNAL			((uint8_t)0x02)     //!< Verify mode: external
#define VERIFY_KEY_P256					((uint8_t)0x04)		//!< Verify key type: P256

#define RANDOM_SEED_UPDATE              ((uint8_t)0x00)     //!< Random mode for automatic seed update
#define SIGN_MODE_EXTERNAL				((uint8_t)0x80)     //!< Sign mode bit 7: external

#define DERIVE_KEY_RANDOM_FLAG          ((uint8_t)4)        //!< DeriveKey 1. parameter; has to match TempKey.SourceFlag

#define BIG_ENDIAN_INT(offset)		((uint16_t)(AUTH.RespBuf[offset] << 8) + AUTH.RespBuf[offset+1])
#define LITTLE_ENDIAN_INT(offset)	((uint16_t)(AUTH.RespBuf[offset+1] << 8) + AUTH.RespBuf[offset])

extern AUTH_DATA	AUTH;

// Function prototypes
void atCRC(uint8_t *crc_buf);
void CheckAuthResponse(void);
void CopyResp(uint8_t *read_buf, uint8_t offset, uint8_t len);
void AuthReadData(void);
void AuthSHA256(uint8_t mode, uint8_t len, uint8_t *data);
void AuthNouce(uint8_t i2c_slv_addr, uint8_t mode);
void AuthVerifyExtern(uint8_t keyid, uint8_t *data);
void AuthRandom(uint8_t mode);
void WaitAuthExecTime(uint8_t wait_time);
void AuthGenDig(void);
void AuthDigestRndNum(void);
void AuthDeriveKey(void);
void AuthWriteData(void);
void AuthReadSlot4Data(void);

#endif /* BATTAUTH_H_ */