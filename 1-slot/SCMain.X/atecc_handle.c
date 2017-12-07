//#include "zAuth_commands.h"
#include "atecc_handle.h"


uint8_t client_serial[ATCA_SERIAL_NUM_SIZE];
uint8_t client_public_key[PUBLIC_KEY_LENGTH];
uint8_t subcon_public_key[PUBLIC_KEY_LENGTH];
uint8_t client_extra_data[EXTRA_DATA_LENGTH];
uint8_t subcon_extra_data[EXTRA_DATA_LENGTH];
uint8_t client_signa_data[SIGNATURE_LENGTH];
uint8_t subcon_signa_data[SIGNATURE_LENGTH];

uint8_t u8DeviceAddress;
ATCAPacket packet;

const uint8_t zebra_seed[] = {0x1C, 0x04, 0x04, 0x00};


/*
//! CheckMAC maximum execution time
#define ECC508_CHECKMAC_EXEC_MAX		((uint8_t) (13))

//! CheckMAC maximum execution time
#define ECC508_COUNTER_EXEC_MAX			((uint8_t) (20))

//! DeriveKey maximum execution time
#define ECC508_DERIVE_KEY_EXEC_MAX		((uint8_t) (50))

//! DeriveKey maximum execution time
#define ECC508_ECDH_EXEC_MAX			((uint8_t) (58))

//! GenDig maximum execution time
#define ECC508_GENDIG_EXEC_MAX			((uint8_t) (11))

//! GenDig maximum execution time
#define ECC508_GENKEY_EXEC_MAX			((uint8_t) (115))

//! HMAC maximum execution time
#define ECC508_HMAC_EXEC_MAX			((uint8_t) (23))

//! Info maximum execution time
#define ECC508_INFO_EXEC_MAX			((uint8_t) (1))

//! Lock maximum execution time
#define ECC508_LOCK_EXEC_MAX			((uint8_t) (32))

//! MAC maximum execution time
#define ECC508_MAC_EXEC_MAX				((uint8_t) (14))

//! Nonce maximum execution time
#define ECC508_NONCE_EXEC_MAX			((uint8_t) (7))

//! Pause maximum execution time
#define ECC508_PAUSE_EXEC_MAX			((uint8_t) (3))

//! PrivWrite maximum execution time
#define ECC508_PRIVWRITE_EXEC_MAX		((uint8_t) (48))

//! Random maximum execution time
#define ECC508_RANDOM_EXEC_MAX			((uint8_t) (23))

//! Read maximum execution time
#define ECC508_READ_EXEC_MAX			((uint8_t) (2))

//! Sign maximum execution time
#define ECC508_SIGN_EXEC_MAX			((uint8_t) (50))

//! SHA maximum execution time
#define ECC508_SHA_EXEC_MAX				((uint8_t) (9))

//! UpdateExtra maximum execution time
#define ECC508_UPDATE_EXEC_MAX			((uint8_t) (10))

//! Verify maximum execution time
#define ECC508_VERIFY_EXEC_MAX			((uint8_t) (58))

//! Write maximum execution time
#define ECC508_WRITE_EXEC_MAX			((uint8_t) (26))
	
*/


const uint8_t exectimes_x08a[] = {   // in milleseconds
	2,                          // WAKE_TWHI
	13,                         // CMD_CHECKMAC
	20,                         // CMD_COUNTER
	50,                         // CMD_DERIVEKEY
	58,                         // CMD_ECDH
	11,                         // CMD_GENDIG
	115,                        // CMD_GENKEY
	23,                         // CMD_HMAC
	1,                          // CMD_INFO
	32,                         // CMD_LOCK
	14,                         // CMD_MAC
	29,                         // CMD_NONCE
	3,                          // CMD_PAUSE
	48,                         // CMD_PRIVWRITE
	24,                         // CMD_RANDOM with SEED Update mode takes ~21ms, high side of range
	2,                          // CMD_READMEM
	9,                          // CMD_SHA
	50,//60,                         // CMD_SIGN
	10,                         // CMD_UPDATEEXTRA
	58,//72,                         // CMD_VERIFY
	26                          // CMD_WRITE
};

extern char cOutString[];

//extern  bool I2C_M_Wr_Full(uint8_t *pdata, uint16_t length, uint16_t address);
//extern  bool I2C_M_Rd_Full(uint8_t *pdata, uint16_t length, uint16_t address);
extern  void Delay_Msec(uint16_t Msec);

I2C1_MESSAGE_STATUS i2c_status;
    
void AteccInit(void){
    memset(client_serial,0,ATCA_SERIAL_NUM_SIZE);
    memset(client_public_key,0,PUBLIC_KEY_LENGTH);
    memset(subcon_public_key,0,PUBLIC_KEY_LENGTH);
    memset(client_extra_data,0,EXTRA_DATA_LENGTH);
    memset(subcon_extra_data,0,EXTRA_DATA_LENGTH);
    memset(client_signa_data,0,SIGNATURE_LENGTH);
    memset(subcon_signa_data,0,SIGNATURE_LENGTH);
}

/**/
#define DISCOVERY_LENGTH (4)
ATCA_STATUS zatwake(void)
{
	uint8_t readdata[DISCOVERY_LENGTH], expected[DISCOVERY_LENGTH] = { 0x04, 0x11, 0x33, 0x43 };
    uint8_t u8Status = 0x00;
    uint8_t u8Retries;
    uint16_t i,j;
    u8Retries = 10;
    while((u8Retries--) > 0 && u8Status == 0x00){
        I2C1_MasterWrite(readdata,1,0,&i2c_status);
        while(i2c_status == I2C1_MESSAGE_PENDING);
        Delay_Msec(2);
        I2C1_MasterRead(readdata,DISCOVERY_LENGTH,u8DeviceAddress,&i2c_status);
        while(i2c_status == I2C1_MESSAGE_PENDING);
    
        if(i2c_status == I2C1_MESSAGE_COMPLETE)
        {
            u8Status = 0x01;
        }
    }
	if ( u8Status == 0x00 )
    {
        puts("wake r err\r");
		return ATCA_COMM_FAIL;
	}
	//if( memcmp( readdata, expected, DISCOVERY_LENGTH ) == 0 )
    if((readdata[0]==0x04)&&(readdata[1]==0x11)&&(readdata[2]==0x33)&&(readdata[3]==0x43))
	//printf("%x %x %x %x\r\n", readdata[0],readdata[1],readdata[2],readdata[3]);	
    return ATCA_SUCCESS;
    puts("wake not match\n\r");
    //printf("%x %x %x %x\n\r",data[0],data[1],data[2],data[3]);
	return ATCA_COMM_FAIL;
    //return ATCA_SUCCESS;
}

ATCA_STATUS zatsend(uint8_t *txdata, int txlength)
{
    uint8_t u8Status = 0;
    // other device types that don't require i/o tokens on the front end of a command need a different hal_i2c_send and wire it up instead of this one
	// this covers devices such as ATSHA204A and ATECCx08A that require a word address value pre-pended to the packet
	// txdata[0] is using _reserved byte of the ATCAPacket
	txdata[0] = 0x03;   // insert the Word Address Value, Command token
	txlength++;         // account for word address value byte.
	
    I2C1_MasterWrite(txdata,txlength,u8DeviceAddress,&i2c_status);
    while(i2c_status == I2C1_MESSAGE_PENDING);
    
    if(i2c_status == I2C1_MESSAGE_COMPLETE)
    {
        u8Status = 0x01;
    }
	if(u8Status) {
		return ATCA_SUCCESS;
	} else {
        puts("send err\r");
		return ATCA_COMM_FAIL;
	}
}

ATCA_STATUS zatreceive(uint8_t *rxdata, uint16_t *rxlength)
{
    uint8_t u8Status = 0;
	uint8_t u8Retries;
    u8Retries = 20;
	
	while((u8Retries--) > 0 && u8Status == 0){
        I2C1_MasterRead(rxdata,*rxlength,u8DeviceAddress,&i2c_status);
        while(i2c_status == I2C1_MESSAGE_PENDING);
    
        if(i2c_status == I2C1_MESSAGE_COMPLETE)
        {
            u8Status = 1;
        }
    }
    if(u8Status) {
		return ATCA_SUCCESS;
	} else {
		puts("rcv err\r");
        return ATCA_COMM_FAIL;
	}
}

ATCA_STATUS zatsleep(void)
{
	uint8_t data[4];
    uint8_t u8Status = 0;
    data[0] = 0x01;  // sleep word address value
    I2C1_MasterWrite(data,1,u8DeviceAddress,&i2c_status);
    while(i2c_status == I2C1_MESSAGE_PENDING);
    
    if(i2c_status == I2C1_MESSAGE_COMPLETE)
    {
        u8Status = 1;
    }
	if ( u8Status == 0)
	{	
        puts("IDLE Err\r");
        return ATCA_COMM_FAIL;
	}
    return ATCA_SUCCESS;
}


ATCA_STATUS zatidle(void)
{
	uint8_t data[4];
    data[0] = 0x02;  // idle word address value
	I2C1_MasterWrite(data,1,u8DeviceAddress,&i2c_status);
    while(i2c_status == I2C1_MESSAGE_PENDING);
    
    if(i2c_status == I2C1_MESSAGE_COMPLETE)
    {
        return ATCA_SUCCESS;
    }
	else{
        return ATCA_NO_DEVICES;
    }
}

/*
#define DISCOVERY_LENGTH (4)
ATCA_STATUS zatwake(void)
{
	uint8_t data[DISCOVERY_LENGTH], expected[DISCOVERY_LENGTH] = { 0x04, 0x11, 0x33, 0x43 };
    bool bStatus = false;
	
	I2C_M_Wr_Full(data, 0, 0);
	
	Delay_Msec(1);
	
	bStatus = I2C_M_Rd_Full(data, DISCOVERY_LENGTH, u8DeviceAddress);

	if ( bStatus == false )
    {
        puts("wake err\r");
		return ATCA_COMM_FAIL;
	}
	if ( memcmp( data, expected, DISCOVERY_LENGTH ) == 0 )
		return ATCA_SUCCESS;
    puts("wake not match\r");
	return ATCA_COMM_FAIL;
}

ATCA_STATUS zatsend(uint8_t *txdata, int txlength)
{
	// other device types that don't require i/o tokens on the front end of a command need a different hal_i2c_send and wire it up instead of this one
	// this covers devices such as ATSHA204A and ATECCx08A that require a word address value pre-pended to the packet
	// txdata[0] is using _reserved byte of the ATCAPacket
	txdata[0] = 0x03;   // insert the Word Address Value, Command token
	txlength++;         // account for word address value byte.
	
	if(I2C_M_Wr_Full(txdata, txlength, u8DeviceAddress) == true) {
		return ATCA_SUCCESS;
	} else {
        puts("send err\r");
		return ATCA_COMM_FAIL;
	}
}

ATCA_STATUS zatreceive(uint8_t *rxdata, uint16_t *rxlength)
{
	bool bStatus = false;
	
	bStatus = I2C_M_Rd_Full(rxdata, *rxlength, u8DeviceAddress);
	if(bStatus == true) {
		return ATCA_SUCCESS;
	} else {
		puts("rcv err\r");
        return ATCA_COMM_FAIL;
	}
}

ATCA_STATUS zatsleep(void)
{
	uint8_t data[4];

	data[0] = 0x01;  // sleep word address value
	if ( I2C_M_Wr_Full(data, 1, u8DeviceAddress) == false)
		return ATCA_COMM_FAIL;
	return ATCA_SUCCESS;
}


ATCA_STATUS zatidle(void)
{
	uint8_t data[4];

	Delay_Msec(1);
    data[0] = 0x02;  // idle word address value
	if (I2C_M_Wr_Full(data, 1, u8DeviceAddress) == false)
		return ATCA_COMM_FAIL;

	return ATCA_SUCCESS;
}

*/

uint16_t atGetExecTime( ATCA_CmdMap cmd )
{
	return (exectimes_x08a[cmd]+1);
}

void atCRC( uint8_t length, uint8_t *data, uint8_t *crc)
{
	uint8_t counter;
	uint16_t crc_register = 0;
	uint16_t polynom = 0x8005;
	uint8_t shift_register;
	uint8_t data_bit, crc_bit;

	for (counter = 0; counter < length; counter++) {
		for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {
			data_bit = (data[counter] & shift_register) ? 1 : 0;
			crc_bit = crc_register >> 15;
			crc_register <<= 1;
			if (data_bit != crc_bit)
				crc_register ^= polynom;
		}
	}
	crc[0] = (uint8_t)(crc_register & 0x00FF);
	crc[1] = (uint8_t)(crc_register >> 8);
}


ATCA_STATUS atCheckCrc(uint8_t *response)
{
	uint8_t crc[ATCA_CRC_SIZE];
	uint8_t count = response[ATCA_COUNT_IDX];

	count -= ATCA_CRC_SIZE;
	atCRC(count, response, crc);

	return (crc[0] == response[count] && crc[1] == response[count + 1]) ? ATCA_SUCCESS : ATCA_BAD_CRC;
}


void atCalcCrc( ATCAPacket *packet )
{
	uint8_t length, *crc;

	length = packet->txsize - ATCA_CRC_SIZE;
	// computer pointer to CRC in the packet
	crc = &(packet->txsize) + length;

	// stuff CRC into packet
	atCRC(length, &(packet->txsize), crc);
}


ATCA_STATUS atRead( ATCAPacket *packet )
{

	// Set the opcode & parameters
	packet->opcode = ATCA_READ;
	packet->txsize = READ_COUNT;

	// variable response size based on read type
	if ((packet->param1 & 0x80) == 0 )
		packet->rxsize = READ_4_RSP_SIZE;
	else
		packet->rxsize = READ_32_RSP_SIZE;

	atCalcCrc( packet );
	return ATCA_SUCCESS;
}

ATCA_STATUS atWrite( ATCAPacket *packet )
{
	int macsize;
	int writesize;

	// Set the opcode & parameters
	packet->opcode = ATCA_WRITE;

	macsize = ( packet->param1 & 0x40 ? 32 : 0 );  // if encrypted, use MAC
	writesize = ( packet->param1 & 0x80 ? 32 : 4 );

	if ( macsize == 32 && writesize == 32 )
		packet->txsize = WRITE_COUNT_LONG_MAC;
	else if ( macsize == 32 && writesize == 4 )
		packet->txsize = WRITE_COUNT_SHORT_MAC;
	else if ( macsize == 0 && writesize == 32 )
		packet->txsize = WRITE_COUNT_LONG;
	else if ( macsize == 0 && writesize == 4 )
		packet->txsize = WRITE_COUNT_SHORT;

	packet->rxsize = WRITE_RSP_SIZE;
	atCalcCrc( packet );
	return ATCA_SUCCESS;
}


ATCA_STATUS atInfo(ATCAPacket *packet)
{
	// Set the opcode & parameters
	packet->opcode = ATCA_INFO;
	packet->txsize = INFO_COUNT;
	packet->rxsize = INFO_RSP_SIZE;

	atCalcCrc( packet );
	return ATCA_SUCCESS;
}

ATCA_STATUS atNonce(ATCAPacket *packet)
{
	// Set the opcode & parameters
	// variable packet size
	packet->opcode = ATCA_NONCE;
	int mode = packet->param1 & 0x03;

	if ( (mode == 0 || mode == 1) ) {       // mode[0:1] == 0 | 1 then NumIn is 20 bytes
		packet->txsize = NONCE_COUNT_SHORT; // 20 byte challenge
		packet->rxsize = NONCE_RSP_SIZE_LONG;
	} else if ( mode == 0x03 ) {            // NumIn is 32 bytes
		packet->txsize = NONCE_COUNT_LONG;  // 32 byte challenge
		packet->rxsize = NONCE_RSP_SIZE_SHORT;
	} else
		return ATCA_BAD_PARAM;

	atCalcCrc( packet );
	return ATCA_SUCCESS;
}

ATCA_STATUS atRandom( ATCAPacket *packet )
{

	// Set the opcode & parameters
	packet->opcode = ATCA_RANDOM;
	packet->txsize = RANDOM_COUNT;
	packet->rxsize = RANDOM_RSP_SIZE;

	atCalcCrc( packet );
	return ATCA_SUCCESS;
}

ATCA_STATUS atGenDig(ATCAPacket *packet, bool hasMACKey)
{

	// Set the opcode & parameters
	packet->opcode = ATCA_GENDIG;

	if ( packet->param1 == 0x03 ) // shared nonce mode
		packet->txsize = GENDIG_COUNT + 32;
	else if ( hasMACKey == true )
		packet->txsize = GENDIG_COUNT_DATA;
	else
		packet->txsize = GENDIG_COUNT;

	packet->rxsize = GENDIG_RSP_SIZE;

	atCalcCrc( packet );
	return ATCA_SUCCESS;
}

ATCA_STATUS atDeriveKey(ATCAPacket *packet, bool hasMAC )
{
	// Set the opcode & parameters
	packet->opcode = ATCA_DERIVE_KEY;

	// hasMAC must be given since the packet does not have any implicit information to
	// know if it has a mac or not unless the size is preset
	switch ( hasMAC ) {
	case true:
		packet->txsize = DERIVE_KEY_COUNT_LARGE;
		break;
	case false:
		packet->txsize = DERIVE_KEY_COUNT_SMALL;
		break;
	}

	packet->rxsize = DERIVE_KEY_RSP_SIZE;
	atCalcCrc( packet );
	return ATCA_SUCCESS;
}

ATCA_STATUS atSHA( ATCAPacket *packet )
{
	if ( packet->param2 > SHA_BLOCK_SIZE )
		return ATCA_BAD_PARAM;

	if ( packet->param1 == 0x01 && packet->param2 != SHA_BLOCK_SIZE )
		return ATCA_BAD_PARAM;                                              // updates should always have 64 bytes of data

	if ( packet->param1 == 0x02 && packet->param2 > SHA_BLOCK_SIZE - 1 )    // END should be 0-63 bytes
		return ATCA_BAD_PARAM;

	// Set the opcode & parameters
	packet->opcode = ATCA_SHA;

	switch ( packet->param1 ) {
	case 0x00: // START
	case 0x03: // PUBLIC
		packet->rxsize = SHA_RSP_SIZE_SHORT;
		packet->txsize = SHA_COUNT_LONG;
		break;
	case 0x01: // UPDATE
		packet->rxsize = SHA_RSP_SIZE_SHORT;
		packet->txsize = SHA_COUNT_LONG + SHA_BLOCK_SIZE;
		break;
	case 0x02: // END
		packet->rxsize = SHA_RSP_SIZE_LONG;
		// check the given packet for a size variable in param2.  If it is > 0, it should
		// be 0-63, incorporate that size into the packet
		packet->txsize = SHA_COUNT_LONG + packet->param2;
		break;
	default:
		return ATCA_BAD_PARAM;	//I'm not implementing HMAC
	}

	atCalcCrc( packet );
	return ATCA_SUCCESS;
}

ATCA_STATUS atVerify( ATCAPacket *packet )
{

	// Set the opcode & parameters
	packet->opcode = ATCA_VERIFY;

	// variable packet size based on mode
	switch ( packet->param1 ) {
	case 0:  // Stored mode
		packet->txsize = VERIFY_256_STORED_COUNT;
		break;
	case 1:  // ValidateExternal mode
		packet->txsize = VERIFY_256_EXTERNAL_COUNT;
		break;
	case 2:  // External mode
		packet->txsize = VERIFY_256_EXTERNAL_COUNT;
		break;
	case 3:     // Validate mode
	case 7:     // Invalidate mode
		packet->txsize = VERIFY_256_VALIDATE_COUNT;
		break;
	default:
		return ATCA_BAD_PARAM;
	}
	packet->rxsize = VERIFY_RSP_SIZE;

	atCalcCrc( packet );
	return ATCA_SUCCESS;
}

ATCA_STATUS atSign( ATCAPacket *packet )
{

	// Set the opcode & parameters
	packet->opcode = ATCA_SIGN;
	packet->txsize = SIGN_COUNT;

	// could be a 64 or 72 byte response depending upon the key configuration for the KeyID
	packet->rxsize = ATCA_RSP_SIZE_64;

	atCalcCrc( packet );
	return ATCA_SUCCESS;
}

ATCA_STATUS isATCAError( uint8_t *data )
{
	uint8_t good[4] = { 0x04, 0x00, 0x03, 0x40 };

	if ( memcmp( data, good, 4 ) == 0 )
		return ATCA_SUCCESS;

	if ( data[0] == 0x04 ) {    // error packets are always 4 bytes long
		switch ( data[1] ) {
		case 0x01:              // checkmac or verify failed
			return ATCA_CHECKMAC_VERIFY_FAILED;
			
		case 0x03: // command received byte length, opcode or parameter was illegal
			return ATCA_BAD_OPCODE;
			
		case 0x0f: // chip can't execute the command
			return ATCA_EXECUTION_ERROR;
			
		case 0x11: // chip was successfully woken up
			return ATCA_WAKE_SUCCESS;
			
		case 0xff: // bad crc found or other comm error
			return ATCA_STATUS_CRC;
			
		default:
			return ATCA_GEN_FAIL;
			
		}
	} else
		return ATCA_SUCCESS;
}

ATCA_STATUS _atcab_exit(void)
{
	return zatidle();
}



ATCA_STATUS atcab_get_addr(uint8_t zone, uint8_t slot, uint8_t block, uint8_t offset, uint16_t* addr)
{
	ATCA_STATUS status = ATCA_SUCCESS;
	uint8_t memzone = zone & 0x03;

	if (addr == NULL)
		return ATCA_BAD_PARAM;
	if ((memzone != ATCA_ZONE_CONFIG) && (memzone != ATCA_ZONE_DATA) && (memzone != ATCA_ZONE_OTP))
		return ATCA_BAD_PARAM;
	do {
		// Initialize the addr to 00
		*addr = 0;
		// Mask the offset
		offset = offset & (uint8_t)0x07;
		if ((memzone == ATCA_ZONE_CONFIG) || (memzone == ATCA_ZONE_OTP)) {
			*addr = block << 3;
			*addr |= offset;
		}else {  // ATCA_ZONE_DATA
			*addr = slot << 3;
			*addr  |= offset;
			*addr |= block << 8;
		}
	} while (0);

	return status;
}

static ATCA_STATUS _cmd_hndlr(ATCAPacket* packet, ATCA_STATUS (*atCmd)( ATCAPacket*), uint8_t cmd)
{
	ATCA_STATUS status = ATCA_GEN_FAIL;;
	uint16_t execution_time = 0;
	if ( (status = atCmd( packet )) != ATCA_SUCCESS )
		return status;
	execution_time = atGetExecTime(  cmd );
	if ( (status = zatwake()) != ATCA_SUCCESS )
    { 
       return status;
    }
    if ( (status = zatsend( (uint8_t*)packet, packet->txsize )) != ATCA_SUCCESS )
    {
       return status;
    }
	// delay the appropriate amount of time for command to execute
	Delay_Msec(execution_time);
	// receive the response
	if ( (status = zatreceive( packet->data, &(packet->rxsize) )) != ATCA_SUCCESS )
	{
       return status;
    }
	// Check response size
	if (packet->rxsize < 4) {
		if (packet->rxsize > 0)
		status = ATCA_RX_FAIL;
		else
		status = ATCA_RX_NO_RESPONSE;
	}
	return status;
}

/*
ATCA_STATUS atcab_read_zone(uint8_t zone, uint8_t slot, uint8_t block, uint8_t offset, uint8_t *data, uint8_t len)
{
	ATCA_STATUS status = ATCA_SUCCESS;
	ATCAPacket packet;
	uint16_t addr;
	uint16_t execution_time = 0;

	do {
		// Check the input parameters
		if (data == NULL)
			return ATCA_BAD_PARAM;

		if ( len != 4 && len != 32 )
			return ATCA_BAD_PARAM;

		// The get address function checks the remaining variables
		if ( (status = atcab_get_addr(zone, slot, block, offset, &addr)) != ATCA_SUCCESS )
			break;

		// If there are 32 bytes to write, then xor the bit into the mode
		if (len == ATCA_BLOCK_SIZE)
			zone = zone | ATCA_ZONE_READWRITE_32;

		// build a read command
		packet.param1 = zone;
		packet.param2 = addr;

		if ( (status = atRead( &packet )) != ATCA_SUCCESS )
			break;

		execution_time = atGetExecTime(CMD_READMEM);

		if ( (status = zatwake()) != ATCA_SUCCESS ) break;

		// send the command
		if ( (status = zatsend((uint8_t*)&packet, packet.txsize) ) != ATCA_SUCCESS )
			break;

		// delay the appropriate amount of time for command to execute
		Delay_Msec(execution_time);

		// receive the response
		if ( (status = zatreceive(packet.data, &(packet.rxsize))) != ATCA_SUCCESS )
			break;

		// Check response size
		if (packet.rxsize < 4) {
			if (packet.rxsize > 0)
				status = ATCA_RX_FAIL;
			else
				status = ATCA_RX_NO_RESPONSE;
			break;
		}

		if ( (status = isATCAError(packet.data)) != ATCA_SUCCESS )
			break;

		memcpy( data, &packet.data[1], len );
	} while (0);

	_atcab_exit();
	return status;
}*/

ATCA_STATUS atcab_read_zone(uint8_t zone, uint8_t slot, uint8_t block, uint8_t offset, uint8_t *data, uint8_t len)
{
	ATCA_STATUS status = ATCA_SUCCESS;
	//ATCAPacket packet;
	uint16_t addr;
    memset(&packet,0,sizeof(packet));
	do {
		// Check the input parameters
		if (data == NULL)
			return ATCA_BAD_PARAM;

		if ( len != 4 && len != 32 )
			return ATCA_BAD_PARAM;

		// The get address function checks the remaining variables
		if ( (status = atcab_get_addr(zone, slot, block, offset, &addr)) != ATCA_SUCCESS )
			break;

		// If there are 32 bytes to write, then xor the bit into the mode
		if (len == ATCA_BLOCK_SIZE)
			zone = zone | ATCA_ZONE_READWRITE_32;

		// build a read command
		packet.param1 = zone;
		packet.param2 = addr;
        status = _cmd_hndlr(&packet, atRead, CMD_READMEM);
        
        if(status != ATCA_SUCCESS) break;
        
        if( (status = atCheckCrc(packet.data))!= ATCA_SUCCESS )
            break;

		if ( (status = isATCAError(packet.data)) != ATCA_SUCCESS )
			break;
  
		memcpy( data, &packet.data[1], len );
	} while (0);

	_atcab_exit();
	return status;
}

ATCA_STATUS atcab_write_zone(uint8_t zone, uint8_t slot, uint8_t block, uint8_t offset, const uint8_t *data, uint8_t len)
{
	ATCA_STATUS status = ATCA_GEN_FAIL;
	//ATCAPacket packet;
	uint16_t addr;
	memset(&packet,0,sizeof(packet));
    
	// Check the input parameters
	if (data == NULL)
		return ATCA_BAD_PARAM;

	if ( len != 4 && len != 32 )
		return ATCA_BAD_PARAM;

	do {
		// The get address function checks the remaining variables
		if ( (status = atcab_get_addr(zone, slot, block, offset, &addr)) != ATCA_SUCCESS )
			break;

		// If there are 32 bytes to write, then xor the bit into the mode
		if (len == ATCA_BLOCK_SIZE)
			zone = zone | ATCA_ZONE_READWRITE_32;

		// build a write command
		packet.param1 = zone;
		packet.param2 = addr;
		memcpy( packet.data, data, len );
        status = _cmd_hndlr(&packet, atWrite, CMD_WRITEMEM);
        
        if ( (status = isATCAError(packet.data)) != ATCA_SUCCESS )
			break;
		
	} while (0);

	_atcab_exit();
	return status;
}

ATCA_STATUS atcab_info( uint8_t *revision )
{
	//ATCAPacket packet;
    memset(&packet,0,sizeof(packet));
    
	ATCA_STATUS status = ATCA_GEN_FAIL;
    
	// build an info command
	packet.param1 = INFO_MODE_REVISION;
	packet.param2 = 0;
    
	do {
		status = _cmd_hndlr(&packet, atInfo, CMD_INFO);
        if(status != ATCA_SUCCESS) break;
        if( (status = atCheckCrc(packet.data))!= ATCA_SUCCESS )
            break;
		if ( (status = isATCAError(packet.data)) != ATCA_SUCCESS )
			break;
        
		memcpy( revision, &packet.data[1], 4 );  // don't include the receive length, only payload
	} while (0);

	if ( status != ATCA_COMM_FAIL )   // don't keep shoving more stuff at the chip if there's something wrong with comm
		_atcab_exit();

	return status;
}

ATCA_STATUS atcab_challenge(const uint8_t *challenge)
{
	ATCA_STATUS status = ATCA_GEN_FAIL;
	//ATCAPacket packet;
	memset(&packet,0,sizeof(packet));
    
	do {
		// Verify the inputs
		if (challenge == NULL) {
			status = ATCA_BAD_PARAM;
			break;
		}

		// build a nonce command (pass through mode)
		packet.param1 = NONCE_MODE_PASSTHROUGH;
		packet.param2 = 0x0000;
		memcpy( packet.data, challenge, 32 );
        status = _cmd_hndlr(&packet, atNonce, CMD_NONCE);
        if(status != ATCA_SUCCESS) break;
		
		if ( (status = isATCAError(packet.data)) != ATCA_SUCCESS )
			break;

	} while (0);

	_atcab_exit();
	return status;
}

ATCA_STATUS atcab_sha_start(void)
{
	ATCA_STATUS status = ATCA_GEN_FAIL;
	//ATCAPacket packet;
	memset(&packet,0,sizeof(packet));
    
	do {// build checkmac command
		packet.param1 = SHA_SHA256_START_MASK;
		packet.param2 = 0;
        
        status = _cmd_hndlr(&packet, atSHA, CMD_SHA);
        if(status != ATCA_SUCCESS) break;
		
		// check for response
		if ( (status = isATCAError(packet.data)) != ATCA_SUCCESS )
			break;
        
	} while (0);

	_atcab_exit();
	return status;
}

ATCA_STATUS sha_public(const uint8_t slot)
{
	ATCA_STATUS status = ATCA_GEN_FAIL;
	//ATCAPacket packet;
    memset(&packet,0,sizeof(packet));
    
	do {

		// build checkmac command
		packet.param1 = SHA_SHA256_PUBLIC_MASK;
		packet.param2 = slot;

		status = _cmd_hndlr(&packet, atSHA, CMD_SHA);
        if(status != ATCA_SUCCESS) break;
        
		// check for response
		if ( (status = isATCAError(packet.data)) != ATCA_SUCCESS )
		break;
        
	} while (0);

	_atcab_exit();
	return status;
}

ATCA_STATUS atcab_sha_update(uint16_t length, const uint8_t *message)
{
	ATCA_STATUS status = ATCA_GEN_FAIL;
	//ATCAPacket packet;
	//uint16_t execution_time = 0;
    memset(&packet,0,sizeof(packet));
    
	do {

		// Verify the inputs
		if ( message == NULL || length > SHA_BLOCK_SIZE ) {
			status = ATCA_BAD_PARAM;
			break;
		}

		// build checkmac command
		packet.param1 = SHA_SHA256_UPDATE_MASK;
		packet.param2 = length;
		memcpy(&packet.data[0], message, length);
        
        status = _cmd_hndlr(&packet, atSHA, CMD_SHA);
        if(status != ATCA_SUCCESS) break;
        
        
		// check for response
		if ( (status = isATCAError(packet.data)) != ATCA_SUCCESS )
			break;
	} while (0);

	_atcab_exit();
	return status;
}

ATCA_STATUS atcab_sha_end(uint8_t *digest, uint16_t length, const uint8_t *message)
{
	ATCA_STATUS status = ATCA_GEN_FAIL;
	//ATCAPacket packet;
	//uint16_t execution_time = 0;
    memset(&packet,0,sizeof(packet));

	if ( length > 63 || digest == NULL )
		return ATCA_BAD_PARAM;

	if ( length > 0 && message == NULL )
		return ATCA_BAD_PARAM;

	do {

		// Verify the inputs
		if ( digest == NULL ) {
			status = ATCA_BAD_PARAM;
			break;
		}

		// build SHA command
		packet.param1 = SHA_SHA256_END_MASK;
		packet.param2 = length;
		if ( length > 0 )
			memcpy(&packet.data[0], message, length);

		status = _cmd_hndlr(&packet, atSHA, CMD_SHA);
        
        if(status != ATCA_SUCCESS) break;
        
		// check for response
		if ( (status = isATCAError(packet.data)) != ATCA_SUCCESS )
			break;
        if( (status = atCheckCrc(packet.data))!= ATCA_SUCCESS )
            break;
		memcpy( digest, &packet.data[ATCA_RSP_DATA_IDX], ATCA_SHA_DIGEST_SIZE );

	} while (0);

	_atcab_exit();
	return status;
}

ATCA_STATUS verify_validate_external(const uint8_t key_id, uint8_t *signature, bool *verified)
{
	ATCA_STATUS status;
	//ATCAPacket packet;
    memset(&packet, 0, sizeof(packet));
    
	do {
		*verified = false;

		// build a verify command
		packet.param1 = VERIFY_MODE_VALIDATEEXTERNAL; //verify the signature
		packet.param2 = key_id;
		memcpy( &packet.data[0], signature, ATCA_SIG_SIZE);

		status = _cmd_hndlr(&packet, atVerify, CMD_VERIFY);

		if(status == ATCA_SUCCESS)
		{
			status = isATCAError(packet.data);
			*verified = (status == 0);
			if (status == ATCA_CHECKMAC_VERIFY_FAILED)
			status = ATCA_SUCCESS; // Verify failed, but command succeeded
		}
	} while (0);

	_atcab_exit();
	return status;
}

ATCA_STATUS dig_random_num()
{
	ATCA_STATUS status = ATCA_GEN_FAIL;
	//ATCAPacket packet;
    memset(&packet, 0, sizeof(packet));
    
	do {
		// build a nonce command (pass through mode)
		packet.param1 = NONCE_MODE_SEED_UPDATE;
		packet.param2 = 0x8000;
		memset( packet.data, 0, 20 );

		status = _cmd_hndlr(&packet, atNonce, CMD_NONCE);
        if(status != ATCA_SUCCESS) break;

		if ((status = isATCAError(packet.data)) != ATCA_SUCCESS) break;
		if( (status = atCheckCrc(packet.data))!= ATCA_SUCCESS )break;
        
	} while (0);

	_atcab_exit();
	return status;
}

ATCA_STATUS atcab_challenge_seed_update( const uint8_t *seed, uint8_t* rand_out )
{
	ATCA_STATUS status = ATCA_GEN_FAIL;
	//ATCAPacket packet;
	memset(&packet, 0, sizeof(packet));
    
	do {
		// Verify the inputs
		if (seed == NULL || rand_out == NULL) {
			status = ATCA_BAD_PARAM;
			break;
		}

		// build a nonce command (pass through mode)
		packet.param1 = NONCE_MODE_SEED_UPDATE;
		packet.param2 = 0x0000;
		memcpy( packet.data, seed, 20 );
        status = _cmd_hndlr(&packet, atNonce, CMD_NONCE);
        if(status != ATCA_SUCCESS) break;

		if ((status = isATCAError(packet.data)) != ATCA_SUCCESS) break;
        if( (status = atCheckCrc(packet.data))!= ATCA_SUCCESS )break;
		memcpy(&rand_out[0], &packet.data[ATCA_RSP_DATA_IDX], 32);

	} while (0);

	_atcab_exit();
	return status;
}

ATCA_STATUS random_seed_update(uint8_t* random_number)
{
	ATCA_STATUS status = ATCA_GEN_FAIL;
	//ATCAPacket packet;
    memset(&packet, 0, sizeof(packet));
    
	do {
		// Verify the inputs
		if (random_number == NULL) {
			status = ATCA_BAD_PARAM;
			break;
		}
		
		// build a nonce command (pass through mode)
		packet.param1 = RANDOM_SEED_UPDATE;
		packet.param2 = 0x0000;

		status = _cmd_hndlr(&packet, atRandom, CMD_RANDOM);//CMD_DERIVEKEY);
        if(status != ATCA_SUCCESS) break;
        
		if ((status = isATCAError(packet.data)) != ATCA_SUCCESS) break;
        if( (status = atCheckCrc(packet.data))!= ATCA_SUCCESS )break;
		
		memcpy(random_number, &packet.data[ATCA_RSP_DATA_IDX], 32);

	} while (0);

	_atcab_exit();
	return status;
}

ATCA_STATUS sign_external(uint16_t slot, uint8_t *signature)
{
	ATCA_STATUS status = ATCA_GEN_FAIL;
	//ATCAPacket packet;
    memset(&packet, 0, sizeof(packet));
    
	do {
		// Verify the inputs
		if (signature == NULL) {
			status = ATCA_BAD_PARAM;
			break;
		}
		
		// build sign command
		packet.param1 = SIGN_MODE_EXTERNAL;
		packet.param2 = slot;

		status = _cmd_hndlr(&packet, atSign, CMD_SIGN);
        if(status != ATCA_SUCCESS) break;
        
		// check for response
		if ( (status = isATCAError(packet.data)) != ATCA_SUCCESS )
		break;
        if( (status = atCheckCrc(packet.data))!= ATCA_SUCCESS )break;
		
		memcpy( signature, &packet.data[1], ATCA_SIG_SIZE );
	} while (0);

	_atcab_exit();
	return status;
}

ATCA_STATUS verify_stored(uint16_t slot, uint8_t *signature)
{
	ATCA_STATUS status = ATCA_GEN_FAIL;
	//ATCAPacket packet;
    memset(&packet, 0, sizeof(packet));
    
	do {
		// Verify the inputs
		if (signature == NULL) {
			status = ATCA_BAD_PARAM;
			break;
		}
		
		// build sign command
		packet.param1 = VERIFY_MODE_STORED;
		packet.param2 = slot;
		memcpy( &packet.data[0], signature, ATCA_SIG_SIZE);
		
		status = _cmd_hndlr(&packet, atVerify, CMD_VERIFY);
        if(status != ATCA_SUCCESS) break;
        
		// check for response
		if ( (status = isATCAError(packet.data)) != ATCA_SUCCESS )
		break;

	} while (0);

	_atcab_exit();
	return status;
}

ATCA_STATUS atcab_gendig_host(uint8_t zone, uint16_t key_id, uint8_t *other_data, uint8_t len)
{
	ATCA_STATUS status = ATCA_GEN_FAIL;
	//ATCAPacket packet;
	uint16_t execution_time = 0;
	bool hasMACKey = 0;
    memset(&packet, 0, sizeof(packet));
    
	do {

		// build gendig command
		packet.param1 = zone;
		packet.param2 = key_id;

		if ( packet.param1 == 0x03 && len == 0x20)
			memcpy(&packet.data[0], &other_data[0], ATCA_WORD_SIZE);
		else if ( packet.param1 == 0x02 && len == 0x20) {
			memcpy(&packet.data[0], &other_data[0], ATCA_WORD_SIZE);
			hasMACKey = true;
		}

		if ( (status = atGenDig( &packet, hasMACKey)) != ATCA_SUCCESS )
			break;

		execution_time = atGetExecTime( CMD_GENDIG);

		if ( (status != zatwake()) != ATCA_SUCCESS ) break;

		// send the command
		if ( (status = zatsend((uint8_t*)&packet, packet.txsize )) != ATCA_SUCCESS )
			break;

		// delay the appropriate amount of time for command to execute
		Delay_Msec(execution_time);

		// receive the response
		if ( (status = zatreceive(packet.data, &(packet.rxsize))) != ATCA_SUCCESS )
			break;

		// Check response size
		if (packet.rxsize < 4) {
			if (packet.rxsize > 0)
				status = ATCA_RX_FAIL;
			else
				status = ATCA_RX_NO_RESPONSE;
			break;
		}

		// check for response
		if ( (status = isATCAError(packet.data)) != ATCA_SUCCESS )
			break;
        

	} while (0);

	_atcab_exit();
	return status;
}

ATCA_STATUS atcab_nonce(const uint8_t *tempkey)
{
	return atcab_challenge(tempkey);
}



ATCA_STATUS derive_key( const uint8_t target_key )
{
	ATCA_STATUS status = ATCA_GEN_FAIL;
	//ATCAPacket packet;
	uint16_t execution_time = 0;

    memset(&packet, 0, sizeof(packet));
    
	do {

		// build a nonce command (pass through mode)
		packet.param1 = DERIVE_KEY_RANDOM_FLAG;
		packet.param2 = target_key;

		if ((status = atDeriveKey( &packet, false)) != ATCA_SUCCESS) break;

		execution_time = atGetExecTime( CMD_DERIVEKEY);

		if ((status = zatwake()) != ATCA_SUCCESS ) break;

		// send the command
		if ( (status = zatsend((uint8_t*)&packet, packet.txsize)) != ATCA_SUCCESS ) break;

		// delay the appropriate amount of time for command to execute
		Delay_Msec(execution_time);

		// receive the response
		if ((status = zatreceive(packet.data, &(packet.rxsize))) != ATCA_SUCCESS) break;

		// Check response size
		if (packet.rxsize < 4) {
			if (packet.rxsize > 0)
			status = ATCA_RX_FAIL;
			else
			status = ATCA_RX_NO_RESPONSE;
			break;
		}

		if ((status = isATCAError(packet.data)) != ATCA_SUCCESS) break;

	} while (0);

	_atcab_exit();
	return status;
}


ATCA_STATUS atcab_read_serial_number(uint8_t* serial_number)
{
	// read config zone bytes 0-3 and 4-7, concatenate the two bits into serial_number
	uint8_t status = ATCA_GEN_FAIL;
	uint8_t bytes_read[ATCA_BLOCK_SIZE];
	uint8_t block = 0;
	uint8_t cpyIndex = 0;
	uint8_t offset = 0;
    uint8_t * temp_point;
    
	do {
		memset(serial_number, 0x00, ATCA_SERIAL_NUM_SIZE);
		// Read first 32 byte block.  Copy the bytes into the config_data buffer
		block = 0;
		offset = 0;
		if ( (status = atcab_read_zone(ATCA_ZONE_CONFIG, 0, block, offset, bytes_read, ATCA_WORD_SIZE)) != ATCA_SUCCESS )
			break;
        temp_point = &(serial_number[cpyIndex]);
		memcpy(temp_point, (int8_t *)(bytes_read), ATCA_WORD_SIZE);
		cpyIndex += ATCA_WORD_SIZE;

		block = 0;
		offset = 2;
		if ( (status = atcab_read_zone(ATCA_ZONE_CONFIG, 0, block, offset, bytes_read, ATCA_WORD_SIZE)) != ATCA_SUCCESS )
			break;
/*point = &(serial_number[cpyIndex];
		memcpy(point, (int8_t *)(bytes_read), ATCA_WORD_SIZE);*/
        temp_point = &(serial_number[cpyIndex]);
		memcpy(temp_point, (uint8_t *)(bytes_read), ATCA_WORD_SIZE);
		cpyIndex += ATCA_WORD_SIZE;

		block = 0;
		offset = 3;
		if ( (status = atcab_read_zone(ATCA_ZONE_CONFIG, 0, block, offset, bytes_read, ATCA_WORD_SIZE)) != ATCA_SUCCESS )
			break;
        temp_point = &(serial_number[cpyIndex]);
		memcpy(temp_point, (uint8_t *)(bytes_read), 1);

	} while (0);

	_atcab_exit();
	return status;
}



bool is_zebra_serial(const uint8_t* serial)
{
	if((serial[0] == 0x01) && (serial[1] == 0x23) && (serial[8] == 0xDE))
		return true;
		
	return false;
}

ATCA_STATUS device_info(uint8_t device)
{
	uint8_t buffer[ATCA_SERIAL_NUM_SIZE];
	ATCA_STATUS status= ATCA_GEN_FAIL;
    
    u8DeviceAddress = device;
	
	status = atcab_info( buffer );	
	if(status == ATCA_SUCCESS) {
		
	} else {
		return status;
	}
		
	status = atcab_read_serial_number(buffer);
	if ( status == ATCA_SUCCESS ) {
		// dump serial num
        if(is_zebra_serial(buffer)== false)
        {
            return ATCA_GEN_FAIL;
        }
        else
        {
            //puts("Serial correct!");
        }
		
	} else {
		return status;
	}
	
	return ATCA_SUCCESS;
}

ATCA_STATUS atcab_read_pubkey(uint8_t slot8toF, uint8_t *pubkey)
{
	uint8_t ret = ATCA_GEN_FAIL;
	uint8_t read_buf[ATCA_BLOCK_SIZE];
	uint8_t block = 0;
	uint8_t offset = 0;
	uint8_t cpyIndex = 0;
	uint8_t cpySize = 0;
	uint8_t readIndex = 0;
    uint8_t * temp_point;
	// Check the pointers
    if (pubkey == NULL)
		return ATCA_BAD_PARAM;
	// Check the value of the slot
	if (slot8toF < 8 || slot8toF > 0xF)
		return ATCA_BAD_PARAM;

	do {
		// The 64 byte P256 public key gets written to a 72 byte slot in the following pattern
		// | Block 1                     | Block 2                                      | Block 3       |
		// | Pad: 4 Bytes | PubKey[0:27] | PubKey[28:31] | Pad: 4 Bytes | PubKey[32:55] | PubKey[56:63] |

		// Read the block
		block = 0;
		if ( (ret = atcab_read_zone(ATCA_ZONE_DATA, slot8toF, block, offset, read_buf, ATCA_BLOCK_SIZE)) != ATCA_SUCCESS )
			break;

		// Copy.  Account for 4 byte pad
		cpySize = ATCA_BLOCK_SIZE - ATCA_PUB_KEY_PAD;
		readIndex = ATCA_PUB_KEY_PAD;
        temp_point = &pubkey[cpyIndex];
		memcpy(temp_point, (uint8_t *)(&read_buf[readIndex]), cpySize);
		cpyIndex += cpySize;

		// Read the next block
		block = 1;
		if ( (ret = atcab_read_zone(ATCA_ZONE_DATA, slot8toF, block, offset, read_buf, ATCA_BLOCK_SIZE)) != ATCA_SUCCESS )
			break;

		// Copy.  First four bytes
		cpySize = ATCA_PUB_KEY_PAD;
		readIndex = 0;
        temp_point = &pubkey[cpyIndex];
		memcpy(temp_point, (uint8_t *)(&read_buf[readIndex]), cpySize);
		cpyIndex += cpySize;
		// Copy.  Skip four bytes
		readIndex = ATCA_PUB_KEY_PAD + ATCA_PUB_KEY_PAD;
		cpySize = ATCA_BLOCK_SIZE - readIndex;
		temp_point = &pubkey[cpyIndex];
        memcpy(temp_point, (uint8_t *)(&read_buf[readIndex]), cpySize);
		cpyIndex += cpySize;

		// Read the next block
		block = 2;
		if ( (ret = atcab_read_zone(ATCA_ZONE_DATA, slot8toF, block, offset, read_buf, ATCA_BLOCK_SIZE)) != ATCA_SUCCESS )
			break;

		// Copy.  The remaining 8 bytes
		cpySize = ATCA_PUB_KEY_PAD + ATCA_PUB_KEY_PAD;
		readIndex = 0;
		temp_point = &pubkey[cpyIndex];
        memcpy(temp_point, (int8_t *)(&read_buf[readIndex]), cpySize);

	} while (0);

	return ret;
}

ATCA_STATUS atcab_write_pubkey(uint8_t slot8toF, uint8_t *pubkey)
{
	ATCA_STATUS status = ATCA_SUCCESS;
	uint8_t write_block[ATCA_BLOCK_SIZE];
	uint8_t block = 0;
	uint8_t offset = 0;
	uint8_t cpyIndex = 0;
	uint8_t cpySize = 0;
	uint8_t writeIndex = 0;

	do {
		// Check the pointers
		if (pubkey == NULL) {
			status = ATCA_BAD_PARAM;
			break;
		}
		// Check the value of the slot
		if (slot8toF < 8 || slot8toF > 0xF) {
			status = ATCA_BAD_PARAM;
			break;
		}
		// The 64 byte P256 public key gets written to a 72 byte slot in the following pattern
		// | Block 1                     | Block 2                                      | Block 3       |
		// | Pad: 4 Bytes | PubKey[0:27] | PubKey[28:31] | Pad: 4 Bytes | PubKey[32:55] | PubKey[56:63] |

		// Setup the first write block accounting for the 4 byte pad
		block = 0;
		writeIndex = ATCA_PUB_KEY_PAD;
		memset(write_block, 0, sizeof(write_block));
		cpySize = ATCA_BLOCK_SIZE - ATCA_PUB_KEY_PAD;
		memcpy(&write_block[writeIndex], &pubkey[cpyIndex], cpySize);
		cpyIndex += cpySize;
		// Write the first block
		status = atcab_write_zone(ATCA_ZONE_DATA, slot8toF, block, offset, write_block, ATCA_BLOCK_SIZE);
		if (status != ATCA_SUCCESS) break;

		// Setup the second write block accounting for the 4 byte pad
		block = 1;
		writeIndex = 0;
		memset(write_block, 0, sizeof(write_block));
		// Setup for write 4 bytes starting at 0
		cpySize = ATCA_PUB_KEY_PAD;
		memcpy(&write_block[writeIndex], &pubkey[cpyIndex], cpySize);
		cpyIndex += cpySize;
		// Setup for write skip 4 bytes and fill the remaining block
		writeIndex += cpySize + ATCA_PUB_KEY_PAD;
		cpySize = ATCA_BLOCK_SIZE - writeIndex;
		memcpy(&write_block[writeIndex], &pubkey[cpyIndex], cpySize);
		cpyIndex += cpySize;
		// Write the second block
		status = atcab_write_zone(ATCA_ZONE_DATA, slot8toF, block, offset, write_block, ATCA_BLOCK_SIZE);
		if (status != ATCA_SUCCESS) break;

		// Setup the third write block
		block = 2;
		writeIndex = 0;
		memset(write_block, 0, sizeof(write_block));
		// Setup for write 8 bytes starting at 0
		cpySize = ATCA_PUB_KEY_PAD + ATCA_PUB_KEY_PAD;
		memcpy(&write_block[writeIndex], &pubkey[cpyIndex], cpySize);
		// Write the third block
		status = atcab_write_zone(ATCA_ZONE_DATA, slot8toF, block, offset, write_block, ATCA_BLOCK_SIZE);
		if (status != ATCA_SUCCESS) break;

	} while (0);

	return status;
}

ATCA_STATUS atcab_read_sig(uint8_t slot8toF, uint8_t *sig)
{
	uint8_t ret = ATCA_GEN_FAIL;
	uint8_t read_buf[ATCA_BLOCK_SIZE];
	uint8_t block = 0;
	uint8_t offset = 0;
	uint8_t cpyIndex = 0;
    uint8_t * temp_point;
    
    do {
		// Check the pointers
		if (sig == NULL) break;

		// Check the value of the slot
		if (slot8toF < 8 || slot8toF > 0xF) break;

		// Read the first block
		block = 0;
		if ( (ret = atcab_read_zone(ATCA_ZONE_DATA, slot8toF, block, offset, read_buf, ATCA_BLOCK_SIZE)) != ATCA_SUCCESS )
			break;

		// Copy.  first 32 bytes
        temp_point = &sig[cpyIndex];
		memcpy(temp_point, (uint8_t *)(&read_buf[0]), ATCA_BLOCK_SIZE);
		cpyIndex += ATCA_BLOCK_SIZE;

		// Read the second block
		block = 1;
		if ( (ret = atcab_read_zone(ATCA_ZONE_DATA, slot8toF, block, offset, read_buf, ATCA_BLOCK_SIZE)) != ATCA_SUCCESS )
			break;

		// Copy.  next 32 bytes
        temp_point = &sig[cpyIndex];
		memcpy(temp_point, (uint8_t *)(&read_buf[0]), ATCA_BLOCK_SIZE);
		cpyIndex += ATCA_BLOCK_SIZE;

	} while (0);

	return ret;
}




#define CHECK_RESPONSE(status, func) do {\
		if(status != ATCA_SUCCESS) {\
			return status;\
		}\
} while(0)

ATCA_STATUS zebra_encrypted_read(uint8_t slot, uint8_t* buffer, uint8_t buffer_length)
{
	ATCA_STATUS status;
	uint8_t block = 0;
	uint8_t i;

	uint8_t random_number[ATCA_BLOCK_SIZE];
	uint8_t enc_digest[ATCA_BLOCK_SIZE];
	
	//64 bytes is 2 blocks
	for(block = 0; block < 2; block++){
		
		u8DeviceAddress = CLIENT_ADDRESS;
	
		//Get a random number
		memset(random_number, 0, sizeof(random_number));	//it might be an improvement to make NumIn be 20 bytes of random data obtained from the MLB RNG.
		status = atcab_challenge_seed_update(random_number, random_number);
		CHECK_RESPONSE(status,atcab_challenge_seed_update);
        
		//Establish encryption key digest in TempKey
		status = atcab_gendig_host(2, 0x0002, (uint8_t *)(&zebra_seed[0]), 0x20);	// Set LEN parameter to 0x20 in order to set hasMAC to true, otherwise zebra_seed won't be transmitted
		CHECK_RESPONSE(status, atcab_gendig_host);
        
		// Read Encrypted
		status = atcab_read_zone(ATCA_ZONE_READWRITE_32|ATCA_ZONE_DATA, slot, block, 0, buffer + (block * ATCA_BLOCK_SIZE), ATCA_BLOCK_SIZE);
		CHECK_RESPONSE(status, atcab_read_zone);
	
		u8DeviceAddress = MLB_ADDRESS;
	
		//Init MLB TempKey to rnadom_number
		status = atcab_nonce(random_number);
		CHECK_RESPONSE(status, atcab_nonce);
	
		//Establish a digest of the random number in TempKey
		status = dig_random_num();
		CHECK_RESPONSE(status, dig_random_num);
	
		//Establish encryption key digest in slot 4 that matches client TempKey
		status = derive_key(4);
		CHECK_RESPONSE(status, derive_key);
	
		//read the 32 byte Encryption Key Digest from Slot 4
		status = atcab_read_zone(ATCA_ZONE_READWRITE_32|ATCA_ZONE_DATA, 4, 0, 0, (uint8_t*)enc_digest, sizeof(enc_digest));

		CHECK_RESPONSE(status, atcab_read_zone);	
		for (i = 0; i < ATCA_BLOCK_SIZE; i++)
			buffer[(block * ATCA_BLOCK_SIZE) + i] = buffer[(block * ATCA_BLOCK_SIZE) + i] ^ enc_digest[i];        
	}	
	return ATCA_SUCCESS;
}

ATCA_STATUS verify_trust(uint8_t pub_key_slot, uint8_t* extra_data, uint8_t* signature)
{
	ATCA_STATUS status;
	bool verified;
	
	//calculate the SHA-256 digest of the Client Sign Message,
	//which is the 64 byte Client Public Key plus the 64 byte ?Extra Data? (from client Slot 13).
	status = atcab_sha_start();
	CHECK_RESPONSE(status, atcab_sha_start);
    //puts("SHA start\r");
	status = sha_public(pub_key_slot);
	CHECK_RESPONSE(status, sha_public);
    //puts("SHA public\r");
    status = atcab_sha_update(EXTRA_DATA_LENGTH, extra_data);
	CHECK_RESPONSE(status, atcab_sha_update);
    //puts("SHA update\r");
    status = atcab_sha_end((uint8_t*)cOutString, 0, NULL);	//Use displaystr as our scratchpad
	CHECK_RESPONSE(status, atcab_sha_end);
    //puts("SHA end\r");
	//Verify of the Client Public Key using the Sub-Contractor Public Key (Slot 12),
	//Client Public Key (Slot 10), the Sign Message Digest (stored in TempKey) and the Client Signature
	status = verify_validate_external(pub_key_slot, signature, &verified);
	CHECK_RESPONSE(status, verify_validate_external);
	
	if(!verified)
	{
		return ATCA_CHECKMAC_VERIFY_FAILED;
	}
	//puts("verified\r");
	return ATCA_SUCCESS;
}

ATCA_STATUS verify_client()
{
	ATCA_STATUS status;
	uint8_t retries;
	
	
	u8DeviceAddress = CLIENT_ADDRESS;
	
	//Verify Serial number
	for(retries = 3; retries; retries--){
		status = atcab_read_serial_number(client_serial);
		if ( status == ATCA_SUCCESS ) {
			// dump serial num
				
			if(is_zebra_serial(client_serial)) 
            {
                //puts("S/N OK\r");
                break;
            }
		}
	}
	if(retries == 0){
		return 0xFF;
	}
	
	//Read Client Public Key from slot 9
	status = atcab_read_pubkey(9, client_public_key);
	CHECK_RESPONSE(status, atcab_read_pubkey);
#ifdef  DEBUG_MSG
    puts("read client pubkey\r");
#endif
	//
	//TODO: Calculate client device Consumption Remaining Count?
	
	//Read Sub-Contractor Public Key from slot 11
	status = atcab_read_pubkey(11, subcon_public_key);
	CHECK_RESPONSE(status, atcab_read_pubkey);
#ifdef  DEBUG_MSG
    puts("read sub pubkey\r");
#endif
    //puts("read sub pubkey\r");
	//TODO: Check against black-list
	
	//read the 64 byte Slot 12 Sub-Contractor Signature from the client device
	status = atcab_read_sig(12, subcon_signa_data);
	CHECK_RESPONSE(status, atcab_read_sig);	
#ifdef  DEBUG_MSG
    puts("read sub sig\r");
#endif
    //puts("read sub sig\r");
    
	//read the 64 byte Slot 10 Client Signature from the client device
	status = atcab_read_sig(10, client_signa_data);
	CHECK_RESPONSE(status, atcab_read_sig);	
#ifdef  DEBUG_MSG
    puts("read client sig\r");
#endif
	//Read the 64 byte Client Extra Data (used to construct the Sign Message) from client device Slot 13 
	status = zebra_encrypted_read(13, client_extra_data, sizeof(client_extra_data));
	CHECK_RESPONSE(status, zebra_encrypted_read);	
#ifdef  DEBUG_MSG
    puts("read client extra\r");
#endif
	//Read the 64 byte Sub-Contractor Extra Data (used to construct the Sign Message) from client device Slot 14
	status = zebra_encrypted_read(14, subcon_extra_data, sizeof(subcon_extra_data));
	CHECK_RESPONSE(status, zebra_encrypted_read);	
#ifdef DEBUG_MSG
    puts("read sub extra\r");
#endif
    u8DeviceAddress = MLB_ADDRESS;
	
	//write the 64 byte Client Public Key (already read in a previous step) to Slot 10 of the MLB device
	status = atcab_write_pubkey(10, client_public_key);
	CHECK_RESPONSE(status, atcab_write_pubkey);
#ifdef DEBUG_MSG
	puts("write client pubkey\r");
#endif	
    //write the 64 byte Sub-Contractor Public Key to Slot 12 of the MLB device
	status = atcab_write_pubkey(12, subcon_public_key);
	CHECK_RESPONSE(status, atcab_write_pubkey);
#ifdef DEBUG_MSG
    	puts("write sub pubkey\r");
#endif	
	
	//calculate the SHA-256 digest of the Sub-Contractor Sign Message, 
	//which is the 64 byte Sub-Contractor Public Key plus the 64 byte ?Extra Data? (from client Slot 14).
	status = verify_trust(12, subcon_extra_data, subcon_signa_data);
	CHECK_RESPONSE(status, verify_trust);
#ifdef DEBUG_MSG
    	puts("verified sub\r");
#endif	
	//calculate the SHA-256 digest of the Client Sign Message, 
	//which is the 64 byte Client Public Key plus the 64 byte ?Extra Data? (from client Slot 13). 
	status = verify_trust(10, client_extra_data, client_signa_data);
	CHECK_RESPONSE(status, verify_trust);
#ifdef DEBUG_MSG
    	puts("verified client\r");
#endif	

	return ATCA_SUCCESS;
}

ATCA_STATUS ecdsa_challenge()
{
	ATCA_STATUS status;
	uint8_t random_number[ATCA_BLOCK_SIZE];
	uint8_t signature[ATCA_SIG_SIZE];
	
	u8DeviceAddress = CLIENT_ADDRESS;
	//cause the client EEPROM RNG Seed to update prior to executing the Sign command (Client)
	status = random_seed_update(random_number);
	CHECK_RESPONSE(status, random_seed_update);
#ifdef DEBUG_MSG
	puts("client seed\r");
#endif
	u8DeviceAddress = MLB_ADDRESS;
	//obtain a 32 byte random number (MLB)
	status = random_seed_update(random_number);
	CHECK_RESPONSE(status, random_seed_update);
#ifdef DEBUG_MSG
	puts("host seed\r");
#endif
	//establish a TempKey value of random number (MLB)
	status = atcab_challenge(random_number);
	CHECK_RESPONSE(status, atcab_challenge);
#ifdef DEBUG_MSG
	puts("host challenge\r");
#endif
    u8DeviceAddress = CLIENT_ADDRESS;
	
	//establish a TempKey value of random number (Client)
	status = atcab_challenge(random_number);
	CHECK_RESPONSE(status, atcab_challenge);
#ifdef DEBUG_MSG
	puts("client challenge\r");
#endif
	//Sign the 32 byte Digest in TempKey (Client)
	status = sign_external(0x0000, signature);
	CHECK_RESPONSE(status, sign_external);
#ifdef DEBUG_MSG
	puts("sign ext\r");
#endif
	u8DeviceAddress = MLB_ADDRESS;
	
	status = verify_stored(0x000A, signature);
	CHECK_RESPONSE(status, verify_stored);
#ifdef DEBUG_MSG
	puts("verify stored\r");
#endif
	return status;
}

ATCA_STATUS atcab_read_slot7(uint8_t *dpointer, uint8_t count)
{
    uint8_t status = ATCA_GEN_FAIL;
    uint8_t bytes_read[ATCA_WORD_SIZE];
    uint8_t slot = 7;
    uint8_t block = 0;
    uint8_t offset = 0;
    u8DeviceAddress = CLIENT_ADDRESS;
    do{
        memset(dpointer, 0x00, count);
        if ( (status = atcab_read_zone(ATCA_ZONE_DATA, slot, block, offset, bytes_read, ATCA_WORD_SIZE)) != ATCA_SUCCESS )
			break;
		memcpy((uint8_t *)dpointer, (uint8_t *)bytes_read, ATCA_WORD_SIZE);
    }while (0);
    _atcab_exit();
	return status;
}

ATCA_STATUS atcab_read_data(uint8_t *datapointer, uint8_t count)
{
	// read config zone bytes 0-3 and 4-7, concatenate the two bits into serial_number
	uint8_t status = ATCA_GEN_FAIL;
	uint8_t bytes_read[ATCA_BLOCK_SIZE];
    uint8_t slot = 8;
	uint8_t block = 4;
	uint8_t offset = 0;
    
    u8DeviceAddress = CLIENT_ADDRESS;
	do {
		//clear target buffer as 0x00;
        memset(datapointer, 0x00, count);
		// Read first 32 byte block.  Copy the bytes into the config_data buffer

		if ( (status = atcab_read_zone(ATCA_ZONE_DATA|ATCA_ZONE_READWRITE_32, slot, block, offset, bytes_read, ATCA_BLOCK_SIZE)) != ATCA_SUCCESS )
			break;
		memcpy((uint8_t *)datapointer, (uint8_t *)bytes_read, ATCA_BLOCK_SIZE);
        block++;
		if ( (status = atcab_read_zone(ATCA_ZONE_DATA|ATCA_ZONE_READWRITE_32, slot, block, offset, bytes_read, ATCA_BLOCK_SIZE)) != ATCA_SUCCESS )
			break;
        datapointer += ATCA_BLOCK_SIZE;
        memcpy((uint8_t *)datapointer, (uint8_t *)bytes_read, ATCA_BLOCK_SIZE);

	} while (0);

	_atcab_exit();
	return status;
}


ATCA_STATUS atca_test(void)
{
    return verify_client();
}

bool Genius_Verification(void)
{
    ATCA_STATUS status;
    status = verify_client();
	if(status != ATCA_SUCCESS) {
		puts("verify fail\r");
		return false;
	}
    else
    {
        status = ecdsa_challenge();
        if(status != ATCA_SUCCESS) {
            puts("ecdsa fail\r");
            return false;
        }
        else
        {
            return true;
        }
    }
}

/**
 End of File
*/