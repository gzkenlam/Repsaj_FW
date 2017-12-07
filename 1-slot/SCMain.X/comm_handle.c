#include "comm_handle.h"


MCP_DATA_TYPE   mcpdata;
CMD_DATA_TYPE   cmd;
BATT_DATA_TYPE battdata;
uint8_t u8BattMode;
CHARGE_DATA_TYPE battspec;
SLOT7_DATA_TYPE battslot7;

//uint8_t u8ReadCF = 0x00;
uint8_t u8DebugOn = 0x00;
uint8_t u8Unhealth = 0x00;
uint8_t u8ManualCalibDebounce = 0x00;

uint16_t u16I2CWait;
uint16_t u16BusWait;

uint8_t DebugFlag;

uint16_t battbuf[8];

uint16_t u16SOHLimit;



#define PACK_END_1  0xCB
#define PACK_END_2  0xBC

#define TXBUF_SIZE  (8)     //size of cmd + 3
#define RXBUF_SIZE  (30)    //size of mcpdata +3

uint8_t u8MACmd[FG_TYPE_AMOUNT] = {MAC_1,MAC_2};
uint8_t u8FgAddr[FG_TYPE_AMOUNT] = {FUELGAUGE_1_ADDRESS,FUELGAUGE_2_ADDRESS};
uint8_t u8FgCmd[FG_TYPE_AMOUNT][9]  =   {FULL_CAP_1, S_O_C_1, CYCLE_COUNT_1, BATT_VOL_1, BATT_CURR_1, TEMPERATURE_1, BATT_STATUS_1, S_O_H_1, BATT_MODE_1,
                                         FULL_CAP_2, S_O_C_2, CYCLE_COUNT_2, BATT_VOL_2, BATT_CURR_2, TEMPERATURE_2, BATT_STATUS_2, S_O_H_2, BATT_MODE_2 };

struct I2C_Packet{
    uint8_t RxBuf[RXBUF_SIZE];
    uint8_t TxBuf[TXBUF_SIZE];
}i2cdata;

uint8_t u8BattDetected = 0;

char cOutString[32];

volatile I2C1_MESSAGE_STATUS i2c_stat;



void Buffer_Init(void)
{
    memset((uint8_t *)(&i2cdata), 0, sizeof(i2cdata));
    memset((uint8_t *)(&mcpdata), 0, sizeof(mcpdata));
    memset((uint8_t *)(&cmd), 0, sizeof(cmd));
    memset((uint8_t *)(&battdata), 0 , sizeof(battdata));
    memset((uint8_t *)(&battspec), 0 , sizeof(battspec));
}

uint8_t MCP_DataRead(void)
{
   //uint8_t u8Retries = 3;

    i2c_stat = I2C1_MESSAGE_PENDING;
    //while((u8Retries-- > 0) && (bStatus == false)){
        //printf("c %x\r\n", u16ChargerStatus);
        I2C1_MasterRead((uint8_t *)(&i2cdata.RxBuf[0]), RXBUF_SIZE, MCP19118_ADDRESS, (I2C1_MESSAGE_STATUS *)(&i2c_stat));
        //printf("b %x\r\n", u16ChargerStatus);
        u16BusWait = 100;
        while(i2c_stat == I2C1_MESSAGE_PENDING && (u16BusWait >0));
        //printf("d %x\r\n", u16ChargerStatus);
        I2C1_Stop(I2C1_MESSAGE_COMPLETE);
        //printf("e %x\r\n", u16ChargerStatus);
        if(i2c_stat == I2C1_MESSAGE_COMPLETE){
             if((i2cdata.RxBuf[0]==RXBUF_SIZE)
            && (i2cdata.RxBuf[RXBUF_SIZE-2] == PACK_END_1)
            && (i2cdata.RxBuf[RXBUF_SIZE-1] == PACK_END_2)){
                //printf("f %x\r\n", u16ChargerStatus);
                memcpy(&mcpdata, &i2cdata.RxBuf[1], sizeof(mcpdata));
                i2cdata.RxBuf[0] = 0; 
                //printf("g %x\r\n", u16ChargerStatus);
                return 1;
             }
             else{
                 return 0;
             }
        }
        else{            
#ifdef DEBUG_MSG
            puts("MCP_I2C_R_ERR\r");
#endif
            return 0;
        }
}

uint8_t MCP_CmdSend(void)
{
    //uint8_t u8Retries = 3;
    uint8_t u8Status = 0;
    memcpy(&i2cdata.TxBuf[1],&cmd,sizeof(cmd));
    memset(&cmd,0,sizeof(cmd));
    //printf("prs %x\r\n", u16ChargerStatus);
    i2cdata.TxBuf[0] = TXBUF_SIZE;
    i2cdata.TxBuf[TXBUF_SIZE-2] = PACK_END_1;
    i2cdata.TxBuf[TXBUF_SIZE-1] = PACK_END_2;
    i2c_stat = I2C1_MESSAGE_PENDING;
    //while((u8Retries-- > 0) && (bStatus == false)){
        I2C1_MasterWrite(&i2cdata.TxBuf[0], TXBUF_SIZE, MCP19118_ADDRESS, (I2C1_MESSAGE_STATUS *)(&i2c_stat));
        u16BusWait = 100; 
        while(i2c_stat == I2C1_MESSAGE_PENDING && (u16BusWait >0));
        
        if(i2c_stat == I2C1_MESSAGE_COMPLETE)
        {
            u8Status = 1;
            u8MCPSendDone = 1;
        }
        else
        {
#ifdef DEBUG_MSG
            puts("MCP_W_ERR\r");   //for debug only
#endif
        }
        I2C1_Stop(I2C1_MESSAGE_COMPLETE);
        //printf("afs %x\r\n", u16ChargerStatus);
    //}//end of while
    return u8Status;
}

void ZQ3_CFbit_Read(void){
    uint8_t u8ZQ3Retry =3;
    uint8_t u8ZQ3Status = 0;
    uint8_t u8ZQ3Sendbuf[4];
    uint8_t u8ZQ3Rcvbuf[3];
    I2C1_MESSAGE_STATUS ZQ3status;
    I2C1_TRANSACTION_REQUEST_BLOCK ZQ3readTRB[2];
    while((u8ZQ3Retry-- > 0)&&(u8ZQ3Status == 0)){
        ZQ3status = I2C1_MESSAGE_PENDING;
        u8ZQ3Sendbuf[0] = 0x00;
        u8ZQ3Sendbuf[1] = 0x56;
        u8ZQ3Sendbuf[2] = 0x00;
    
        I2C1_MasterWrite(&u8ZQ3Sendbuf[0],3 , FUELGAUGE_2_ADDRESS, &ZQ3status);
        u16BusWait = 100; 
        while(ZQ3status == I2C1_MESSAGE_PENDING && (u16BusWait >0));
        if(ZQ3status == I2C1_MESSAGE_COMPLETE){
            //puts("send good\r");
            //Delay_Msec(2);
        }
        else{
            //puts("send err\r");
            break;
        }
        u8ZQ3Sendbuf[0] = 0x3E;
        I2C1_MasterWriteTRBBuild(   &ZQ3readTRB[0],
                                    u8ZQ3Sendbuf,
                                    1,
                                    FUELGAUGE_2_ADDRESS);
    // Build TRB for receiving data
        I2C1_MasterReadTRBBuild(    &ZQ3readTRB[1],
                                    &u8ZQ3Rcvbuf[0],
                                    3,
                                    FUELGAUGE_2_ADDRESS);
    
        if(ZQ3status != I2C1_MESSAGE_FAIL){
            I2C1_MasterTRBInsert(2, ZQ3readTRB, &ZQ3status);
            //u16BusWait = 50;
            while((ZQ3status == I2C1_MESSAGE_PENDING));// && (u16BusWait > 0));
            if(ZQ3status == I2C1_MESSAGE_COMPLETE){
                u8ZQ3Status = 1;
                //printf("%2x %2x %2x\r",u8ZQ3Rcvbuf[0],u8ZQ3Rcvbuf[1],u8ZQ3Rcvbuf[2]);
                u8BattMode = u8ZQ3Rcvbuf[2];
                Delay_Msec(8);
                break;
            }
            else{
                
#ifdef DEBUG_MSG
                printf("err=%d\r",status);
#else
                Delay_Msec(1);
#endif
                puts("read CF fail\r\n");
                I2C1_Stop(I2C1_MESSAGE_COMPLETE);
            }
            
        }
    }    
}

void ZQ5_CFbit_Read(void){
    uint8_t u8Retry =3;
    uint8_t u8Status = 0;
    uint8_t u8Sendbuf[4];
    uint8_t u8Rcvbuf[4];
    I2C1_MESSAGE_STATUS status;
    I2C1_TRANSACTION_REQUEST_BLOCK readTRB[2];
    while((u8Retry-- > 0)&&(u8Status == 0)){
        status = I2C1_MESSAGE_PENDING;
        u8Sendbuf[0] = MAC_1;
        u8Sendbuf[1] = 2;
        u8Sendbuf[2] = 0x56;
        u8Sendbuf[3] = 0x00;
    
        I2C1_MasterWrite(&u8Sendbuf[0],4 , FUELGAUGE_1_ADDRESS, &status);
        u16BusWait = 100; 
        while(status == I2C1_MESSAGE_PENDING && (u16BusWait >0));
        if(status == I2C1_MESSAGE_COMPLETE){
            //puts("send good\r");
            //Delay_Msec(1);
        }
        else{
            //puts("send err\r");
            break;
        }
   
    u8Sendbuf[0] = MAC_1;
    I2C1_MasterWriteTRBBuild(   &readTRB[0],
                                &u8Sendbuf[0],
                                1,
                                FUELGAUGE_1_ADDRESS);
    // Build TRB for receiving data
    I2C1_MasterReadTRBBuild(    &readTRB[1],
                                &u8Rcvbuf[0],
                                4,
                                FUELGAUGE_1_ADDRESS);
    
        if(status != I2C1_MESSAGE_FAIL){
            I2C1_MasterTRBInsert(2, readTRB, &status);
            //u16BusWait = 50;
            while((status == I2C1_MESSAGE_PENDING));// && (u16BusWait > 0));
            if(status == I2C1_MESSAGE_COMPLETE){
                u8Status = 1;
                //printf("%d %2x %2x %2x\r",u8Rcvbuf[0],u8Rcvbuf[1],u8Rcvbuf[2],u8Rcvbuf[3]);
                u8BattMode = u8Rcvbuf[3];
                Delay_Msec(5);
            }
            else{
                
#ifdef DEBUG_MSG
                printf("err=%d\r",status);
#else
                Delay_Msec(1);
#endif
                puts("read CF fail\r\n");
                I2C1_Stop(I2C1_MESSAGE_COMPLETE);
            }            
        }
    }    
}

uint8_t FuelGauge_DataRead(uint16_t *pData, uint8_t slaveDeviceAddress, uint8_t u8Cmd)
{
    uint8_t u8Status = 0;
    uint8_t u8Retry = 3;
    I2C1_MESSAGE_STATUS status;
    I2C1_TRANSACTION_REQUEST_BLOCK readTRB[2];
    uint8_t     writeBuffer[2];
    
    // this initial value is important
    status = I2C1_MESSAGE_PENDING;
    // build the write buffer first
    // starting address of the EEPROM memory
    writeBuffer[0] = u8Cmd;
    // we need to create the TRBs for a random read sequence to the EEPROM
    // Build TRB for sending address
    I2C1_MasterWriteTRBBuild(   &readTRB[0],
                                writeBuffer,
                                1,
                                slaveDeviceAddress);
    // Build TRB for receiving data
    I2C1_MasterReadTRBBuild(    &readTRB[1],
                                (uint8_t *)pData,
                                2,
                                slaveDeviceAddress);
    while((u8Retry-- > 0)&&(u8Status == 0)){
        if(status != I2C1_MESSAGE_FAIL){
            I2C1_MasterTRBInsert(2, readTRB, &status);
            //u16BusWait = 50;
            while((status == I2C1_MESSAGE_PENDING));// && (u16BusWait > 0));
            if(status == I2C1_MESSAGE_COMPLETE){
                u8Status = 1;
                break;
            }
            else{
                
#ifdef DEBUG_MSG
                printf("err=%d\r",status);
#else
                Delay_Msec(1);
#endif
                I2C1_Stop(I2C1_MESSAGE_COMPLETE);
            }
            
        }
    } 
    return u8Status;
}

/*
bool FuelGauge_DataRead(uint16_t *pData, uint8_t u8I2C_Addr, uint8_t u8Cmd)
{
    bool bStatus = false;
    uint8_t u8Retry = 1;
    uint8_t u8Temp;
    uint16_t u16RetVal;
    u8Temp = u8Cmd;
    while((u8Retry-- > 0)&&(bStatus == false)){
        I2C1_MasterWrite(&u8Temp, 1, u8I2C_Addr, (I2C1_MESSAGE_STATUS *)(&i2c_stat));
        u16BusWait = 20;
        while(i2c_stat == I2C1_MESSAGE_PENDING);// && (u16BusWait > 0));
        Delay_Msec(1);
        if(i2c_stat == I2C1_MESSAGE_COMPLETE){
            I2C1_MasterRead((uint8_t *)(&u16RetVal), 2, u8I2C_Addr, (I2C1_MESSAGE_STATUS *)(&i2c_stat));
            u16BusWait = 20;
            while(i2c_stat == I2C1_MESSAGE_PENDING);// && (u16BusWait > 0));
            if(i2c_stat == I2C1_MESSAGE_COMPLETE){
                bStatus = true;
                if(u16RetVal != 0xffff){
                    *pData = u16RetVal;
                }
            }
            else{
                //I2C1_ErrHandle();
#ifdef  DEBUG_MSG
                puts("FG_R_ERR\r");
#endif
                
            }
        }
        else{
            //I2C1_ErrHandle();
#ifdef  DEBUG_MSG
            puts("FG_W_ERR\r");      
#endif
            
        }        
    }//end of while
    return bStatus;
}
*/

uint8_t Batt_DataRead(uint8_t j)
{
    uint8_t i;
    uint8_t u8Size;
    uint16_t *pData;
    pData = &battbuf[0];
    if(battdata.u8Status == ECC508_VALID){
        u8Size = 8;
        if(u8FgAddr[j] == FUELGAUGE_2_ADDRESS){
            ZQ3_CFbit_Read();
        }
        else{
            ZQ5_CFbit_Read();
        }
    }
    else{
        u8Size = 7;
        battbuf[7] = 0xff;
        //battbuf[8] = 0;
    }
    for(i=0;i<u8Size;i++){
        if(FuelGauge_DataRead(pData, u8FgAddr[j], u8FgCmd[j][i]))
        {
            pData++;
        }
        else
        {
            return 0;
        }
        Delay_Msec(1);
    }
    return 1;
}

void Charge_Setting(void)
{

    if(battdata.u8Status == ECC508_NOT_VALID){
        battspec.u16ChargeHighTempK = FG_HIGH_TEMPK;
        battspec.u16ChargeLowTempK = FG_LOW_TEMPK;
        battspec.u16MaxCurr = FG_CC_CURRMA;
        battspec.u16SCLRate1 = FG_CC_CURRMA;
        battspec.u16SCLTempK1 = FG_HIGH_TEMPK;
        battspec.u16SCLVolt1 = FG_CV_VOLTMV;
        battspec.u8ChargeTC = FG_TC_CURRMA;
        battspec.u16PrechargeThreshold = FG_PC_THRESHOLD;
        battspec.u8RechargeThreshold = FG_RC_THRESHOLD;
        battspec.u8SlowChargeTimeout = FG_SLOW_TIMEOUT;
        battspec.u8FastChargeTimeout = FG_FAST_TIMEOUT;
#ifdef DEBUG_MSG
        if((battdata.u16Capacity >3100) && (battdata.u16Capacity < 4000)){
            battspec.u16MaxCurr = 1600;
        }
#endif
        
    }
    if(battdata.u8Status == ECC508_VALID){
        battspec.u16SCLVolt1 *= 10;
        battspec.u16SCLVolt2 *= 10;
        battspec.u16SCLVolt3 *= 10;
        if(battspec.u16ChargeHighTempK > (2730+450)){
            battspec.u16ChargeHighTempK = 2730+450;
        }        
    }
    /*if(battdata.u8Status == OTHER_BATT){
        puts("batt_no_support\r\n");
        not support batt without FG
        if(mcpdata.u16MCPStatus & MCP_CALIBED){
            battspec.u16MaxCurr = mcpdata.u16Calib800mA + mcpdata.u16CalibZeroA;
            battspec.u16PrechargeThreshold = (mcpdata.u16Calib800mV<<3)-mcpdata.u16Calib800mV;
            battspec.u16SCLVolt1 = mcpdata.u16Calib8V4;
            battspec.u8ChargeTC = (mcpdata.u16Calib800mA>>3) + mcpdata.u16CalibZeroA;
        }
        else{
            puts("no_support\r\n");
        }
        
    }*/
    //limitation of max current for device
    if(battspec.u16MaxCurr > MAX_CURRENT_MA){
        battspec.u16MaxCurr = MAX_CURRENT_MA;
    }
    //charge cycle and SoH verification
    if(battdata.u8Status == ECC508_VALID){
        if((battslot7.u8DTL >0) && (battslot7.u8DTL <=100)){
            u16SOHLimit = battslot7.u8DTL;
        }
        else{
            u16SOHLimit = DEFAULT_SOH_LIMIT;
        }     
    }
    else{
        u16SOHLimit = 50;
    }
    u8Unhealth = 0;
    /*
    if(battdata.u16Cycle >= DEFAULT_CYCLE_LIMIT){
        u8Unhealth = 0x01;
    }
    else{
        if(battdata.u16SOH < u16SOHLimit){
            u8Unhealth = 0x01;
        }
        else{
            u8Unhealth = 0;
        }
    }
    */
}

/*
 *  Call every time as MCP_Read(), DeJitter
 */
uint8_t u8DetCounter;
uint8_t u8EjtCounter; 
void BattDetect_DeJitter(void)
{
    if(MCP_DataRead()){
        u16CommTimeOut = 0;
        if(u8BattDetected){
            if(mcpdata.u16MCPStatus & MCP_BATT_DET){
                u8EjtCounter = 0;
            }
            else{
                u8DetCounter = 0;
                u8EjtCounter ++;
            }
            if(u8EjtCounter > 0){
                u8EjtCounter = 0;
                u8BattDetected = 0;
                //ResetDevice();
            }
        }
        else{
            if(mcpdata.u16MCPStatus & MCP_BATT_DET){
                u8DetCounter++;
                u8EjtCounter = 0;
            }
            else{
                u8DetCounter = 0;
                /*
                if((mcpdata.u16Therm > NTC_HOT_CHARGE_ADC) &&(mcpdata.u16Therm < NTC_COOL_CHARGE_ADC))
                {
                    if(mcpdata.u16MCPStatus == (MCP_DETECT_ON|MCP_PWMOUT_ON|MCP_CALIBED))
                    {
                        Send_MCPCmd(SET_STATUS,MCP_DETECT_ON,MCP_BATT_DET);
                    }
                }
                 */
            }
            if(u8DetCounter > 6){
                u8DetCounter = 0;
                u8BattDetected = 1;
            }
        }
    }
}



uint8_t u8RdErrCount = 0;
void Batt_Comm(void)
{
    uint8_t u8Buffer[4];
    uint8_t u8Retry = 3;
    //bool bStatus = false;
    uint8_t u8Status = 0;
    uint8_t u8Temp = 0;
    if(battdata.u8Status)
    {
#ifdef DEBUG_MSG
        Debug_Mode(DEBUG_FUELGAUGE_STATUS);
#endif        
        //Battery Type detected, and battery valid
        if(u8BattDetected)           
        {
            /*
            //for Alpha build only skip data query after fully charge
            if(u16ChargerStatus & FULL_CHARGE){
                return;
            }
            */
            //data query from fuel gauge
            if(battdata.u8FgAddr < FG_TYPE_AMOUNT)
            {                  
                if(!Batt_DataRead(battdata.u8FgAddr)){
                    //memset((uint8_t *)(&battdata), 0, sizeof(battdata));
                    //memset((uint8_t *)(&battspec), 0, sizeof(battspec));
                    if(u8RdErrCount++ > 3){
                        u8BattDetected = 0;
                        battdata.u8Status = 0;
                        u8RdErrCount = 0;
                        //ResetDevice();
                    }
                }
                else{
                    memcpy((uint8_t *)(&battdata.u16Capacity), (uint8_t *)(&battbuf[0]), sizeof(battdata));
                    //if CF is set, SOH = 100
                    if(u8BattMode &0x80){
                        battdata.u16SOH = 100;
                    }
                    //added in v1.15 v1.17
                    //Unhealth detection
                    if(battdata.u8Status == ECC508_VALID){
                        //PP+ battery
                        if((battdata.u16SOH < u16SOHLimit)
                           && (battdata.u16Cycle > 5)){
                            u8Unhealth = 0x01;
                        }
                    }
                    else{
                        if(battdata.u16Cycle >= DEFAULT_CYCLE_LIMIT){
                            u8Unhealth = 0x01;
                        }    
                    }
                    
                    u8RdErrCount = 0;
                }

            }//end of if Fuel Gauge is detected
            else{
                //others;
                Set_LED(FBLINK_RED);
                while(1);
            }
            return;
        }//end of if(u8BattDetected)
        else{
            //Battery removed? Clear data and charge spec
            //memset((uint8_t *)(&battdata), 0, sizeof(battdata));
            //memset((uint8_t *)(&battspec), 0, sizeof(battspec));
            battdata.u8Status = 0;      //to save rom size
            battspec.u16MaxCurr = 0;    //to save rom size
        }
    }//end of if(u8Batt.u8Status)
    else{
#ifdef DEBUG_MSG
        if(DebugFlag == DEBUG_FUELGAUGE_STATUS){
            Debug_Mode(DEBUG_MESS_OFF);
        }
#endif 
        //Battery Type is not detected 
        if(u8BattDetected)
        {
            //battery is valid in slot
            //fuel gauge detection
            //BT_MODE_SetHigh();
            //Set_LED(BURST_AMBER);
 
            //fuel gauge detection
            u8Temp = 0;
            u8Status = 0;
            u8Retry = 3;
            while((u8Temp < FG_TYPE_AMOUNT)&& (u8Status == 0)){
                while(u8Retry-->0 && u8Status == 0){
                    I2C1_MasterRead(u8Buffer, 4, u8FgAddr[u8Temp], (I2C1_MESSAGE_STATUS *)(&i2c_stat));
                    while(i2c_stat == I2C1_MESSAGE_PENDING);            
                    if(i2c_stat == I2C1_MESSAGE_COMPLETE){
                        u8Status = 1;
                        battdata.u8FgAddr = u8Temp;
#ifdef  DEBUG_MSG
                        printf("FG Found: %x\r\n",u8FgAddr[battdata.u8FgAddr]);
#else
                        Delay_Msec(2);
#endif
                    }
                }//end of while(u8Retry-->0 && bStatus == false)
                u8Temp++;
            }//end of while(battdata.u8FgAddr<2)
            

            if(u8Status == 0)
            {
#ifdef  DEBUG_MSG
                puts("FG not Found\r\n");
#endif
                battdata.u8FgAddr = u8Temp;
            }
            if(battdata.u8FgAddr < FG_TYPE_AMOUNT){
                //FuelGauge detected, read a series of data
                battdata.u8Status = ECC508_NOT_VALID;
                if(Batt_DataRead(battdata.u8FgAddr)){
                    memcpy((uint8_t *)(&battdata.u16Capacity), (uint8_t *)(&battbuf[0]), sizeof(battdata));

                }
                else{
                    //batt data read error, reboot
                    Set_LED(FBLINK_RED);
                    while(1);
                }
            }//end of if FG is detected
            else{
                //There is no FG in Batt
                battdata.u8Status = OTHER_BATT;
                puts("batt_no_support\r\n");
                return;
            }
            //ATECC508 detection
            AteccInit();            //add for eliminate auth. bug
            u8Status = 0;
            u8Retry = 3;
            while(u8Retry-->0 && u8Status == 0){
                I2C1_MasterRead(u8Buffer, 4, CLIENT_ADDRESS, (I2C1_MESSAGE_STATUS *)(&i2c_stat));
                while(i2c_stat == I2C1_MESSAGE_PENDING);            
                if(i2c_stat == I2C1_MESSAGE_COMPLETE){
                    u8Status = 1;
                    battdata.u8Status = ECC508_VALID;
#ifdef  DEBUG_MSG
                    puts("ECC508_VALID\r\n");
#else
                    Delay_Msec(1);
#endif
                }
                else{
                    battdata.u8Status = ECC508_NOT_VALID;
#ifdef  DEBUG_MSG
                    puts("ECC508_NOT_VALID\r\n");
#else
                    Delay_Msec(1);
#endif
                }
            }//end of while
            //load charge settings
            if(battdata.u8Status == ECC508_VALID){
                u8Retry = 3;
                u8Status = 0;
                while((u8Retry-- > 0) && (u8Status == 0)){
                    u16CommTimeOut = 0;
                    if(Genius_Verification()){
#ifdef  DEBUG_MSG
                        puts("Auth_done\r\n");
#else
                        Delay_Msec(1);
#endif
                        u8Retry = 3;
                        while((u8Retry-- > 0) && (u8Status == 0)){
                            if(atcab_read_data((uint8_t *)(&battspec), sizeof(battspec)) == ATCA_SUCCESS){
                                if(atcab_read_slot7((uint8_t *)(&battslot7), sizeof(battslot7)) == ATCA_SUCCESS){
                                    u8Status = 1;
                                }                                
#ifdef  DEBUG_MSG
                                puts("Slot8 R\r\n");
#endif                                
                            }
                            else{
#ifdef  DEBUG_MSG
                                puts("Slot8 R ERR\r\n");
#endif                      
                                Delay_Msec(1);
                            }                            
                        }// end of slot 8 reading loop
                    }
                    else{
#ifdef  DEBUG_MSG
                        puts("Auth_fail\r\n");
#endif    
                        Delay_Msec(1);
                    }    
                }//end of while((u8Retry-- > 0) && (bStatus == false))
                if(u8Status == 0){
                    battdata.u8Status = ECC508_NOT_VALID;
                }
            }//end of if(battdata.u16Status == ECC508_VALID)
                            
            Charge_Setting();
#ifdef DEBUG_MSG
            printf("bstat:%d,fgaddr:%d\n\r",battdata.u8Status,battdata.u8FgAddr);
#endif
        }//if mcp_batt_det
        else{
            //no battery valid in slot
            u16TotalChargeSec = 0;
            
            memset((uint8_t *)(&battdata),0x00, sizeof(battdata));
            memset((uint8_t *)(&battspec),0x00, sizeof(battspec));
            s16MaxChargeCurr = 0;
            s16MaxChargeVolt = 0;
            u8BattMode = 0;
            u16SOHLimit = 0;
            return;
        }
    }//end of not if(battdata.u8Status)
}


void USART_Handle(void)
{
    char ctemp;
    if(eusartRxCount>0)
    { 
        ctemp = getch();
        switch(ctemp)
        {
            case '1':{
                Debug_Mode(DEBUG_MESS_OFF);
                puts(CLEAR_SCREEN_CMD);
                break;
            }
#ifdef DEBUG_MSG
            case '2':{
                puts("read_batt_0\n\r");
                memset(battbuf,0,sizeof(battbuf));
                if(Batt_DataRead(0)){
                printf("%d %d %d %d %d %d %d \n\r",battbuf[0],battbuf[1],battbuf[2],battbuf[3],battbuf[4],battbuf[5],battbuf[6] );}
                else{
                    puts("ReadFail\n\r");
                }
                break;
            }
            case '3':{
                puts("read_batt_1\n\r");
                memset(battbuf,0,sizeof(battbuf));
                if(Batt_DataRead(1)){
                printf("%d %d %d %d %d %d %d \n\r",battbuf[0],battbuf[1],battbuf[2],battbuf[3],battbuf[4],battbuf[5],battbuf[6] );}
                else{
                    puts("ReadFail\n\r");
                }
                 break;
            }
#endif
            case 'D':{
                Debug_Mode(DEBUG_ALL_STATUS);
                break;
            }
            case 'd':{
                Debug_Mode(DEBUG_CHARGE_STATUS);
                break;
            }
            case '4':{
                if(u8LEDOverridden){
                    u8LEDStatus = SOLID_GREEN;
                }
                break;
            }
            case '5':{
                if(u8LEDOverridden){
                    u8LEDStatus = SOLID_AMBER;
                }
                break;
            }
            case '6':{
                if(u8LEDOverridden){
                    u8LEDStatus = SOLID_RED;
                }
                break;
            }
            case '7':{
                if(u8LEDOverridden){
                    u8LEDStatus = BURST_GREEN;
                }
                break;
            }
            case '8':{
                if(u8LEDOverridden){
                    u8LEDStatus = BURST_AMBER;
                }
                break;
            }
            case '9':{
                if(u8LEDOverridden){
                    u8LEDStatus = FBLINK_RED;
                }
                break;
            }
            case '0':{
                if(u8LEDOverridden){
                    u8LEDStatus = LED_OFF;
                }
                break;
            }
            case 'L':;
            case 'l':{
                u8LEDOverridden ^= 0x01;
                if(u8LEDOverridden){
                    puts("LED ov\r\n");
                }
                else{
                    puts("LED nov\r\n");
                }
                break;
            }            
            case 'C':;     
            case 'c':{
                if(u8ManualCalibDebounce ++ >2){
                    Send_MCPCmd(CLR_CALIB,0,0);
                    u8ManualCalibDebounce = 0;
                }
                break;
            }
            case 'M':;
            case 'm':{
                u8DebugOn ^= 0x01;
                if(u8DebugOn){
                    puts("DEBUG ON\r\n");
                }
                else{
                    puts("DEBUG OFF\r\n");
                }
                break;
            }
            case 'r':{
                break;
            }
            case 'Y':;
            case 'y':{
                User_Yes_Interface();
                break;
            }
            case 'n':;
            case 'N':{
                User_No_Interface();
                break;
            }
            case 'v':;
            case 'V':{
                Show_Version();
                break;
            }
            case 's':;
            case 'S':{
                Show_SampleData();
                break;
            }
            case ' ':{
                break;
            }
            case '+':{
                if(u8DebugOn){
                    if((battdata.u8Status>0)&&(battdata.u8Status<OTHER_BATT)){
                        if(battspec.u16MaxCurr<2500){
                            battspec.u16MaxCurr+= 100;
                        }
                        else{
                            battspec.u16MaxCurr = 2500;
                        }
                    }
                }
                break;
            }
            case '-':{
                if(u8DebugOn){
                    if((battdata.u8Status>0)&&(battdata.u8Status<OTHER_BATT)){
                        if(battspec.u16MaxCurr>900){
                            battspec.u16MaxCurr -= 100;
                        }
                        else{
                            battspec.u16MaxCurr = 900;
                        }
                    }    
                }                
                break;
            }
            default: {putch(ctemp); break;}
        }//end of switch(ctemp)
        return;
    }
    else
    {
        return;
    }
}



void Debug_Mode(uint8_t disp)
{
    DebugFlag = disp;
}

void ChgStat_Printf(void)
{
    switch(u16ChargerStatus&0xff00){
        case CALIBING:{
            printf("Cal_step:%d\r\n", u8CalibStep);
            break;
        }
        case PRE_CHARGE:{
            puts("Pre-Charge  \r\n");
            break;
        }
        case CC_CHARGE:{
            puts("Const Curr  \r\n");
            break;
        }
        case CV_CHARGE:{
            puts("Const Volt  \r\n");
            break;
        }
        case FULL_CHARGE:{
            puts("Full Charge \r\n");
            break;
        }
        default:
            printf("default:%4x\r\n",u16ChargerStatus);
            break;
    }
}

void Show_Version(void){
    printf("%s%s",CLEAR_SCREEN_CMD,MOVE_CURSOR_13_00_MSG);
    printf("%s%s",SAVE_CURSOR_CMD,MOVE_CURSOR_01_01_MSG);
    printf("Jasper 1-Slot Battery Charger FW Ver.%d.%d\r\n",FW_VERSION,FW_BUILD);
    printf("%s", RESTORE_CURSOR_CMD);            
}

void Show_SampleData(void){
    printf("CSTA=%x,BSTA=%x,Batt=%d,SOC=%d,Volt=%d,Curr=%d,Temp=%d,Time=%d,Err=%x\r\n",mcpdata.u16MCPStatus, battdata.u16Status, battdata.u8Status, battdata.u16SOC,battdata.s16Vol,battdata.s16Cur,battdata.u16TempK, u16TotalChargeSec, mcpdata.u16ErrorStatus);
}

void Debug_Display(void)
{
    switch (DebugFlag){
        case DEBUG_MCP_STATUS:{
            
            break;
        }
        case DEBUG_FUELGAUGE_STATUS:{
            Show_SampleData();
            break;
        }
        case DEBUG_ALL_STATUS:{
            puts(CLEAR_SCREEN_CMD);
            
        }
        case DEBUG_CHARGE_STATUS:{
            printf("%s%s",SAVE_CURSOR_CMD,MOVE_CURSOR_03_01_MSG);
            //Row 1
            printf("STAT: %4x   ", mcpdata.u16MCPStatus);
            printf("Err:  %4x   ", mcpdata.u16ErrorStatus);
            printf("OVCC: %4d   ", mcpdata.u8OVCC);
            printf("OVCF: %4d   ", mcpdata.u8OVCF);
            printf("%s", CRLF_MSG);
            //Row 2
            printf("Vin:  %4d   ", mcpdata.u16Vin);
            printf("VBat: %4d   ", mcpdata.u16VBat);
            printf("IOut: %4d   ", mcpdata.u16IOut);
            printf("Rt:   %4d", mcpdata.u16Therm);
            printf("%s", CRLF_MSG);
            //Row 3
            printf("Cal_I:%4d   ", mcpdata.u16Calib800mA);
            printf("Cal_V:%4d   ", mcpdata.u16Calib800mV);
            printf("Cal_B:%4d   ", mcpdata.u16Calib8V4);
            printf("Cal_Z:%4d   ", mcpdata.u16CalibZeroA);
            printf("MCP_V:%d.%d", (mcpdata.u8Version&0xf0)>>4,mcpdata.u8Version&0x0f);
            printf("%s", CRLF_MSG);
            //Row 4
            printf("SOH:  %4d   ", battdata.u16SOH);
            printf("SOC:  %4d   ", battdata.u16SOC);
            printf("Volt: %4d   ", battdata.s16Vol);
            printf("Curr: %4d   ", battdata.s16Cur);
            printf("CF: %4x", u8BattMode);
            printf("%s", CRLF_MSG);
            //Row 5
            printf("Stat: %4x   ", battdata.u16Status);
            printf("Cap:  %4d   ", battdata.u16Capacity);
            printf("Temp: %4d   ", battdata.u16TempK);
            printf("Cycl: %4d   ", battdata.u16Cycle);
            printf("%s", CRLF_MSG);
            //Row 6
            printf("Htemp: %4d   ", battspec.u16ChargeHighTempK);
            printf("Ltemp: %4d   ", battspec.u16ChargeLowTempK);
            printf("MaxCr: %4d   ", s16MaxChargeCurr);
            printf("MaxVl: %4d   ", s16MaxChargeVolt);
            printf("%s", CRLF_MSG);
            //Row 7
            printf("PC_Vl: %4d   ", battspec.u16PrechargeThreshold);
            printf("TC_Cr: %4d   ", battspec.u8ChargeTC);
            printf("STime: %4d   ", battspec.u8SlowChargeTimeout);
            printf("FTime: %4d   ", battspec.u8FastChargeTimeout);
            printf("RC: %2d      ", battspec.u8RechargeThreshold);
            printf("%s", CRLF_MSG);
            //Row 8
            printf("Sl_Hr: %4d   ", u8SlowTimeHr );
            printf("Sl_Sc: %4d   ", u16SlowTimeSec );
            printf("Fs_Hr: %4d   ", u8FastTimeHr );
            printf("Fs_Sc: %4d   ", u16FastTimeSec );
            printf("DTP/H: %2d/%2d ", u16SOHLimit, u8Unhealth);
            printf("%s", CRLF_MSG);
            //Row 9
            ChgStat_Printf();
            
            printf("%s", RESTORE_CURSOR_CMD);
            break;
        }
        case DEBUG_MESS_OFF:
        default:break;
    }
}

/**
 End of File
*/