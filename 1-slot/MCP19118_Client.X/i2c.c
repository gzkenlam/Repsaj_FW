/*
 * I2C communication management for 
 */

#include "i2c.h"

#define I2C_SLAVE_ADDRESS 0x15 
#define I2C_SLAVE_MASK    0x7F

typedef enum
{
    SLAVE_NORMAL_DATA,
    SLAVE_DATA_ADDRESS,
} SLAVE_WRITE_DATA_TYPE;

/**
 Section: Global Variables
*/

volatile uint8_t    I2C_slaveWriteData      = 0x55;

bool    bI2C_Received = false;

bool    bI2C_Transmitted = true;

struct I2C_Packet{
    uint8_t RxByteCount;
    uint8_t RxBuf[RXBUF_SIZE];
    uint8_t TxByteCount;
    uint8_t TxBuf[TXBUF_SIZE];
}i2cdata;

SET_VALUE  set;

/**
 Section: Local Functions
*/
void I2C_StatusCallback(I2C_SLAVE_DRIVER_STATUS i2c_bus_state);



/**
  Prototype:        void I2C_Initialize(void)
  Input:            none
  Output:           none
  Description:      I2C_Initialize is an
                    initialization routine that takes inputs from the GUI.
  Comment:          
  Usage:            I2C_Initialize();

*/
void I2C_Initialize(void)
{
    // initialize the hardware
    // R_nW write_noTX; P stopbit_notdetected; S startbit_notdetected; BF RCinprocess_TXcomplete; SMP High Speed; UA dontupdate; CKE disabled; D_nA lastbyte_address; 
    SSPSTAT = 0x00;
    // SSPEN enabled; WCOL no_collision; CKP disabled; SSPM 7 Bit Polling; SSPOV no_overflow; 
    SSPCON1 = 0x26;
    // ACKEN disabled; GCEN disabled; PEN disabled; ACKDT acknowledge; RSEN disabled; RCEN disabled; ACKSTAT received; SEN disabled; 
    SSPCON2 = 0x00;
    // ACKTIM ackseq; SBCDE disabled; BOEN disabled; SCIE disabled; PCIE disabled; DHEN disabled; SDAHT 300ns; AHEN disabled; 
    SSPCON3 = 0x08;
    // SSPMSK 127; 
    SSPMSK = (I2C_SLAVE_MASK << 1);  // adjust UI mask for R/nW bit            
    // SSPADD 8; 
    SSPADD = (I2C_SLAVE_ADDRESS << 1);  // adjust UI address for R/nW bit

    // clear the slave interrupt flag
    PIR1bits.SSPIF = 0;
    // enable the master interrupt
    PIE1bits.SSPIE = 1;
    
    memset(&i2cdata,0,sizeof(i2cdata));
}

void I2C_ISR ( void )
{
    uint8_t     i2c_data                = 0x55;


    // NOTE: The slave driver will always acknowledge
    //       any address match.
    PIR1bits.SSPIF = 0;        // clear the slave interrupt flag
    i2c_data        = SSPBUF;  // read SSPBUF to clear BF
    if(1 == SSPSTATbits.R_nW)
    {
        if((1 == SSPSTATbits.D_nA) && (1 == SSPCON2bits.ACKSTAT))
        {
            // callback routine can perform any post-read processing
            I2C_StatusCallback(I2C_SLAVE_READ_COMPLETED);
        }
        else
        {
            // callback routine should write data into SSPBUF
            I2C_StatusCallback(I2C_SLAVE_READ_REQUEST);
        }
    }
    else if(0 == SSPSTATbits.D_nA)
    {
        // this is an I2C address

        // callback routine should prepare to receive data from the master
        I2C_StatusCallback(I2C_SLAVE_WRITE_REQUEST);
    }
    else
    {
        I2C_slaveWriteData   = i2c_data;

        // callback routine should process I2C_slaveWriteData from the master
        I2C_StatusCallback(I2C_SLAVE_WRITE_COMPLETED);
    }
    SSPCON1bits.CKP = 1;    // release SCL
} // end I2C_ISR()

/*
void CRCCalc( uint8_t length, uint8_t *data, uint8_t *crc)
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
   *crc = (uint8_t)(crc_register & 0x00FF);
   crc++;
   *crc = (uint8_t)(crc_register >> 8);
}

bool CRCCheck(uint8_t length, uint8_t *data)
{
    uint8_t crc_temp[2];
    CRCCalc(length, *data, crc_temp);
    if((crc_temp[0]==data[length])&&(crc_temp[1]==data[length+1]))
    {
        return true;
    }
    else
    {
        return false;
    }
}
*/

/*

    Example implementation of the callback

    This slave driver emulates an EEPROM Device.
    Sequential reads from the EEPROM will return data at the next
    EEPROM address.

    Random access reads can be performed by writing a single byte
    EEPROM address, followed by 1 or more reads.

    Random access writes can be performed by writing a single byte
    EEPROM address, followed by 1 or more writes.

    Every read or write will increment the internal EEPROM address.

    When the end of the EEPROM is reached, the EEPROM address will
    continue from the start of the EEPROM.
*/

void I2C_StatusCallback(I2C_SLAVE_DRIVER_STATUS i2c_bus_state)
{
    //static uint8_t *pWriteData;
    static uint8_t slaveWriteType   = SLAVE_NORMAL_DATA;
    
    switch (i2c_bus_state)
    {
        case I2C_SLAVE_WRITE_REQUEST:
            // the master will be sending the eeprom address next
            //slaveWriteType  = SLAVE_DATA_ADDRESS;
            i2cdata.RxByteCount = RXBUF_SIZE;
            i2cdata.TxByteCount = TXBUF_SIZE;
            break;


        case I2C_SLAVE_WRITE_COMPLETED:
            
            if(i2cdata.RxByteCount)
            {
                i2cdata.RxBuf[RXBUF_SIZE-i2cdata.RxByteCount] = I2C_slaveWriteData;
                i2cdata.RxByteCount --;
                if(i2cdata.RxByteCount == 0){
                    //receive done
                    if( (i2cdata.RxBuf[0] == RXBUF_SIZE)
                        &&  (i2cdata.RxBuf[RXBUF_SIZE-2] == PACK_END_1)
                        && (i2cdata.RxBuf[RXBUF_SIZE-1] == PACK_END_2)
                      ){
                        bI2C_Received = true;
                        mcp.u16MCPStatus |= MCP_GETCMD;
                        u16CommTimeOut = 0;
                        
                    }
                    else{
                        bI2C_Received = false;   
                    }
                }
            }  
            /*
            switch(slaveWriteType)
            {
                case SLAVE_DATA_ADDRESS:
                    //eepromAddress   = I2C_slaveWriteData;
                    i2cdata.RxByteCount = I2C_slaveWriteData;
                    i2cdata.RxBuf[0] = I2C_slaveWriteData;
                    i2cdata.RxByteCount --;
                    i2cdata.TxByteCount = TXBUF_SIZE;
                    break;


                case SLAVE_NORMAL_DATA:
                default:
                    
                    if(i2cdata.RxByteCount)
                    {
                        i2cdata.RxBuf[RXBUF_SIZE-i2cdata.RxByteCount] = I2C_slaveWriteData;
                        i2cdata.RxByteCount --;
                        if(i2cdata.RxByteCount == 0)
                        {
                            //receive done
                            if( (i2cdata.RxBuf[0] == RXBUF_SIZE)
                            //&&  (i2cdata.RxBuf[RXBUF_SIZE-2] == PACK_END_1)
                            //&& (i2cdata.RxBuf[RXBUF_SIZE-1] == PACK_END_2)
                               ){
                                //bI2C_Received = true;
                                mcp.u16MCPStatus |= MCP_GETCMD;
                                cal.u16Calib800mA = i2cdata.RxBuf[4];//i2cdata.RxBuf[RXBUF_SIZE-2];
                                cal.u16Calib800mV = i2cdata.RxBuf[5];//i2cdata.RxBuf[RXBUF_SIZE-1];
                                WDFEED_Toggle();
                            }
                            else{
                               bI2C_Received = false;   
                            }
                        }
                    }   
                    break;
            } // end switch(slaveWriteType)

            slaveWriteType  = SLAVE_NORMAL_DATA;*/
            break;

        case I2C_SLAVE_READ_REQUEST:
            if((i2cdata.TxByteCount>0) && (bI2C_Transmitted == false))
            {   
                i2cdata.TxByteCount --;
                SSPBUF = i2cdata.TxBuf[(TXBUF_SIZE - i2cdata.TxByteCount - 1)];
				if(i2cdata.TxByteCount == 0)
            	{
                	bI2C_Transmitted = true;
            	}
            }
            else
            {
                i2cdata.TxByteCount = 0;
				bI2C_Transmitted = true;
                SSPBUF = 0xff;
                
            }
            break;

        case I2C_SLAVE_READ_COMPLETED:
        default:
            if(i2cdata.TxByteCount == 0)
            {
                //i2cdata.TxByteCount = TXBUF_SIZE;
                bI2C_Transmitted = true;
            }
            
    } // end switch(i2c_bus_state)

}

void I2C_Data_Handler(void)
{
    uint8_t i;
    if(bI2C_Received)
    {
        memcpy((uint8_t *)(&set.u8SetCmd),(uint8_t *)(&i2cdata.RxBuf[1]), sizeof(set));
        i2cdata.RxBuf[0] = 0;   //clear buffer
        switch (set.u8SetCmd)
        {
            case NONE_CMD:{
                mcp.u16MCPStatus &= ~MCP_GETCMD;
                break;
            }
            case INIT_CMD:{
                //exit initial mode to run
                mcp.u16MCPStatus &= ~MCP_INIT;
                break;
            }
            case TURN_ON:{
                if(!(mcp.u16MCPStatus & MCP_ERROR))
                {
                    Turn_On_Output();
                }
                break;
            }
            case TURN_OFF:{
                Turn_Off_Output();
                break;
            }
            case PWM_ON:{
                if(!(mcp.u16MCPStatus & MCP_ERROR)){
                    PWMOUT_On();                    
                }
                break;
            }
            case PWM_OFF:{
                PWMOUT_Off();
                break;
            }
            case CHGSW_ON:{
                CHGSW_On();
                break;
            }
            case CHGSW_OFF:{
                CHGSW_Off();
                break;
            }
            case INC_CURR:{
                if(mcp.u16MCPStatus & MCP_PWMOUT_ON)
                {
                    Inc_Iout();
                }
                break;
            }
            case DEC_CURR:{
                if(mcp.u16MCPStatus & MCP_PWMOUT_ON)
                {
                    Dec_Iout();
                }
                break;
            }
            case INC_HIGH_CURR:{
                if(mcp.u16MCPStatus & MCP_PWMOUT_ON)
                {
                    for(i=0;i<20;i++){
                        Inc_Iout();
                    }
                }
                break;
            }
            case DEC_HIGH_CURR:{
                if(mcp.u16MCPStatus & MCP_PWMOUT_ON)
                {
                    for(i=0;i<20;i++){
                        Dec_Iout();
                    }
                }
                break;
            }
            case VOLT_REG_ON:{
                if((mcp.u16MCPStatus & MCP_PWMOUT_ON) && (mcp.u16MCPStatus & MCP_CHGSW_ON))
                {
                    mcp.u16MCPStatus |= MCP_VOLT_REG;
                    mcp.u16MCPStatus &= ~MCP_VOLT_STABLE;
                    mcp.u16VoltGoal = set.u16SetValue1;
                }
                break;
            }
            case VOLT_REG_OFF:{
                mcp.u16MCPStatus &= ~MCP_VOLT_REG;
                break;
            }
            case CURR_REG_ON:{
                if((mcp.u16MCPStatus & MCP_PWMOUT_ON) && (mcp.u16MCPStatus & MCP_CHGSW_ON))
                {
                    mcp.u16MCPStatus |= MCP_CURR_REG;
                    mcp.u16MCPStatus &= ~MCP_CURR_STABLE;
                    mcp.u16CurrGoal = set.u16SetValue1;
                }
                break;
            }
            case CURR_REG_OFF:{
                mcp.u16MCPStatus &= ~MCP_CURR_REG;
                break;
            }
            case CURR_REG_OUT:{
                if(!(MCP_ERROR & mcp.u16MCPStatus))
                {
                    Turn_On_Output();
                    mcp.u16MCPStatus |= MCP_CURR_REG;
                    mcp.u16MCPStatus &= ~MCP_CURR_STABLE;
                    mcp.u16CurrGoal = set.u16SetValue1;
                }
                break;
            }
            case VOLT_REG_OUT:{
                if(!(MCP_ERROR & mcp.u16MCPStatus))
                {
                    Turn_On_Output();
                    mcp.u16MCPStatus |= MCP_VOLT_REG;
                    mcp.u16MCPStatus &= ~MCP_VOLT_STABLE;
                    mcp.u16VoltGoal = set.u16SetValue1;
                }
                break;
            }
            case SET_ERROR:{
                //clear error with SetValue 1
                mcp.u16ErrorStatus &= ~set.u16SetValue1;
                //set error with SetValue 2
                mcp.u16ErrorStatus |= set.u16SetValue2;
                if(mcp.u16ErrorStatus)
                {
                    mcp.u16MCPStatus |= MCP_ERROR;
                }
                break;
            }
            case SET_STATUS:{
                //clear error with SetValue 1
                mcp.u16MCPStatus &= ~set.u16SetValue1;
                //set error with SetValue 2
                mcp.u16MCPStatus |= set.u16SetValue2;
                break;
            }
            case SET_CALIB:{
                if((cal.u16Calib800mA > MIN_CALIB_800MA) &&
                        (cal.u16Calib800mA < MAX_CALIB_800MA) &&
                        (cal.u16Calib800mV > MIN_CALIB_800MV) &&
                        (cal.u16Calib800mV < MAX_CALIB_800MV) &&
                        (cal.u16Calib8V4 > MIN_CALIB_8V4) &&
                        (cal.u16Calib8V4 < MAX_CALIB_8V4) ){
                    mcp.u16MCPStatus |= MCP_CALIBED;
                }
                break;
            }
            case CLR_CALIB:{
                //clear CALIB bit for start calibration
                mcp.u16MCPStatus &= ~MCP_CALIBED;
                break;
            }
            case CALIB_800MA:{
                cal.u16Calib800mA = set.u16SetValue1;
                break;
            }
            case CALIB_800MV:{
                cal.u16Calib800mV = set.u16SetValue1;
                break;
            }
            case CALIB_8V4:{
                cal.u16Calib8V4 = set.u16SetValue1;
                break;
            }
            case CALIB_ZEROA:{
                cal.u16CalibZeroA = set.u16SetValue1;
                break;
            }
            case WR_CALIB:{
                Write_Flash(CAL_BASE_ADDR, (uint8_t *)(&cal.u16Calib800mA));
                break;
            }
            case RD_CALIB:{
                Read_Calib();
                if((cal.u16Calib800mA > MIN_CALIB_800MA) &&
                        (cal.u16Calib800mA < MAX_CALIB_800MA) &&
                        (cal.u16Calib800mV > MIN_CALIB_800MV) &&
                        (cal.u16Calib800mV < MAX_CALIB_800MV) &&
                        (cal.u16Calib8V4 > MIN_CALIB_8V4) &&
                        (cal.u16Calib8V4 < MAX_CALIB_8V4) ){
                    mcp.u16MCPStatus |= MCP_CALIBED;
                }
                break;
            }
            case SET_REMOTE:{
                mcp.u16MCPStatus |= MCP_REMOTE;
                break;
            }
            case CLR_REMOTE:{
                mcp.u16MCPStatus &= ~MCP_REMOTE;
                break;
            }            
            case SET_THERM_ERR:{
                mcp.u16ErrorStatus |= THERM_OVER;
                mcp.u16MCPStatus |= MCP_ERROR;
                break;
            }
            case CLR_THERM_ERR:{
                mcp.u16ErrorStatus &= ~THERM_OVER;
                break;
            }
            case SET_TIMER_ERR:{
                mcp.u16ErrorStatus |= SLOW_TIME_OUT;
                mcp.u16MCPStatus  |= MCP_ERROR;
                break;
            }
            default:break;
        }
        memset(&i2cdata.RxByteCount,0,RXBUF_SIZE+1);
        bI2C_Received = false;
        //refresh status after setting if there is a command set
        bI2C_Transmitted = true;
    }
    if(bI2C_Transmitted)
    {
        memcpy((uint8_t *)(&i2cdata.TxBuf[1]),(uint8_t *)(&mcp), sizeof(mcp));
        memcpy((uint8_t *)(&i2cdata.TxBuf[sizeof(mcp)+1]),(uint8_t *)(&adc), sizeof(adc));
        memcpy((uint8_t *)(&i2cdata.TxBuf[sizeof(mcp)+sizeof(adc)+1]),(uint8_t *)(&cal), sizeof(cal));
        i2cdata.TxBuf[TXBUF_SIZE-2] = PACK_END_1;
        i2cdata.TxBuf[TXBUF_SIZE-1] = PACK_END_2;
		i2cdata.TxBuf[0]= TXBUF_SIZE;
        mcp.u16MCPStatus &= ~MCP_GETCMD;
        i2cdata.TxByteCount = TXBUF_SIZE;       
        bI2C_Transmitted = false;
        //WDFEED_SetLow();
    }
    return;
}
