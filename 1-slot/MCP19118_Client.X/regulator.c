/*
 * File:   regulator.c
 * Author: KLam
 *
 * Created on November 18, 2016, 2:29 PM
 */
#include "regulator.h"

REGULATOR_VALUE  mcp;

uint16_t u16BattDetTimeoutCounter;
uint16_t u16BattDetTimer;
//uint16_t u16BattVoltLow;
//uint16_t u16BattVoltHigh;


void Regulator_Init(void)
{
    //Switch off PWM 
    ATSTCON = 0x81;     //bit7:reserved, 
                        //bit6-5:unimplemented, 
                        //bit4:reserved, 
                        //bit3:0 HIDIS high-side driver is enable
                        //bit2:0 LODIS low-side driver is enable
                        //bit1:0 BNCHEN GPA0 is not configured for analog bench test output
                        //bit0:1 DRVDIS high and low side driver set low, PHASE pin is floating
    /* PWM Generator */
    T2CON = 0x00;   //initial TMR2 controller
    TMR2 = 0x00;    //initial TMR2 
    PWMPHL = 0x00;  //SLAVE Phase Shift Byte = 0;
    PWMRL = 0x19;   // 0x19 Max duty cycle = PWMRL*Tosc*T2prescaler 
    PR2 = 0x1B;     // 0x1B 300 kHz = Fosc/PR2
    T2CON = 0x04;   // TMR2ON:1 T2CKPS:00, Enable TMR2 , prescaler is 1 
    
    //Enable diode emulation. This is a must or the battery could destroy
    PE1 = _PE1_DECON_MASK;
    /*
    PE1bits.DECON = 1;  //Diode emulation mode enabled
    PE1bits.DVRSTR = 0; //High-side 2A source/sink drive strengh
    PE1bits.HDLYBY = 0; //High-side dead time bypass is disabled
    PE1bits.LDLYBY = 0; //Low-side dead time bypass is disabled
    PE1bits.PDEN = 0;   //-Vsen week pull down is disabled
    PE1bits.PUEN = 0;   //+Vsen week pull up is disabled
    PE1bits.UVTEE = 0;  //Output undervoltage accelerator is disabled
    PE1bits.OVTEE = 0;  //Output overvoltage accelerator is disabled
    */
    /* DC-DC parameter setting */
    DEADCON = 0xff; //0x9b; //HDLY:1001 = 47ns, LDLY:1011= 48ns 
    ABECON = 0x0f;  //OVDCEN:0 output ov DAC is disable
                    //UVDCEN:0 output uv DAC is disable
                    //MEASEN:0 relative efficiency measurement is not in progress
                    //SLCPBY:0 slope compensation is enable
                    //CRTMEN:1 current measurement circuitry is enable
                    //TMPSEN:1 internal temperature sensor and overtemperature is enable
                    //RECIREN:1 relative efficiency measurement circuitry is enable
                    //PATHEN:1 signal chain circuitry is enable
    VZCCON = 0x60; /* 3.28mV per step, -105.12mV, Voltage for zero current */
    CMPZCON = 0x0f; //0x0D ; //0x0F; /* Compensation Freq is 1500Hz, Gain is 0 dB*/
    SLPCRCON = 0xf0; /* Slope compensation amplitude 1.25V Vpp*/
    CSGSCON = 0x0d; /* AC gain 19.0dB */
    CSDGCON = 0x00; /* DC gain 19.5dB */
    //output over current protection
    OCCON = 0xFF;   /* Output overcurrent setting 0x66 */
                    //OCEN:1 output oc DAC is enabled
                    //OCLEB:11 780ns blanking
                    //OOC:11111: 625mV drop
    //Shall be enable in ABECON first
    //Output overvoltage protection, can be use for over current
    OOVCON = 0x0E;  // Set value =OV level/0.015 = 11,  4A*0.01*2000/510 =
    //Output undervoltage protection, no use
    OUVCON = 0x0B;  // Set value = UV level/0.015
    //Shall be enable in ABECON bit7
    OVCCON = 0x00; /* Zero output voltage ovc = (Vout/0.0158)-1 */
    OVFCON = 0x80; /* Enable DAC and zero output voltage output voltage DAC enable */
    BUFFCON = 0x01; /* Benchtest Device set as stand-alone, Error amp output plus slope compensation,input to PWM comparator */
    
    VINLVL = 0x80|0x19;  //bit7: enable, 0x1A = 26.5*ln(10.66/4) 0x19 = 26.5ln(10.28/4)
    VINIF = 0;
    VINIE = 0;
    OCIE = 0;
    OCIF = 0;
    OVIE = 0;
    OVIF = 0;
    
}



void Inc_Iout(void) {
    if (mcp.u8OVCC >= MAX_OVCCON)
        return;

    mcp.u8OVCF++;
    if (mcp.u8OVCF > IOUT_LSB_ROLL) {
        mcp.u8OVCF = 0;

        if (mcp.u8OVCC < MAX_OVCCON)
            mcp.u8OVCC++;
        else
            mcp.u8OVCC = MAX_OVCCON;
    }

    OVFCON = 0x80 | mcp.u8OVCF;
    OVCCON = mcp.u8OVCC;
}

void Dec_Iout(void) {
    if ((mcp.u8OVCC == 0) && (mcp.u8OVCF == 0))
        return;

    mcp.u8OVCF--;
    if (mcp.u8OVCF > IOUT_LSB_ROLL) {
        mcp.u8OVCF = IOUT_LSB_ROLL;

        if (mcp.u8OVCC > 0)
            mcp.u8OVCC--;
        else
            mcp.u8OVCC = 0;
    }

    OVFCON = 0x80 | mcp.u8OVCF;
    OVCCON = mcp.u8OVCC;
}

void Turn_On_Output(void)
{
    mcp.u8OVCF = 0;
    mcp.u8OVCC = MIN_OVCCON;
    OVCCON = mcp.u8OVCC;
    OVFCON = 0x80|mcp.u8OVCF; 
    PWMOUT_On();     //PWM on
    CHGSW_On();
}

void Turn_Off_Output(void)
{
    CHGSW_Off();
    PWMOUT_Off();     //PWM off
    OVCCON = 0;
    OVFCON = 0x80;
    mcp.u8OVCF = 0;
    mcp.u8OVCC = 0;
}

void Regulation_Curr(void)
{
    static uint8_t u8StableCounter;
    uint16_t u16TempCurr;
    if((mcp.u16MCPStatus & MCP_PWMOUT_ON) && (mcp.u16MCPStatus & MCP_CHGSW_ON) && (mcp.u16MCPStatus & MCP_CURR_REG))
    {
        u16TempCurr = adc.u16IOut;
        if(u16TempCurr < (mcp.u16CurrGoal))     //!!! 0-1 = 65535
        {
            Inc_Iout();
        }
        else if(u16TempCurr > (mcp.u16CurrGoal + 2))
        {
            Dec_Iout();
        }
        else
        {
            if(u8StableCounter < 20)
            {
                u8StableCounter++;
            }
            else
            {
                mcp.u16MCPStatus |= MCP_CURR_STABLE;
            }
        }
    }
    else
    {
        mcp.u16CurrGoal = 0;
        u8StableCounter = 0;
        mcp.u16MCPStatus &= ~MCP_CURR_REG;
        mcp.u16MCPStatus &= ~MCP_CURR_STABLE;
    }
}

void Regulation_Volt(void)
{
    static uint8_t u8StableCounter = 0;
    if((mcp.u16MCPStatus & MCP_PWMOUT_ON) && (mcp.u16MCPStatus & MCP_CHGSW_ON) && (mcp.u16MCPStatus & MCP_VOLT_REG))
    {
        
        if((adc.u16VBat) < (mcp.u16VoltGoal)) //!!! 0-1 = 65535
        {
            Inc_Iout();
        }
        else 
        {
            if (adc.u16VBat > (mcp.u16VoltGoal+2)){
                Dec_Iout();
            }
            else{
                u8StableCounter ++;
                if(u8StableCounter > 10){
                     mcp.u16MCPStatus |= MCP_VOLT_STABLE;
                     u8StableCounter = 0;
                }
            }
        }
    }
    else
    {
        mcp.u16VoltGoal = 0;
        u8StableCounter = 0;
        mcp.u16MCPStatus &= ~MCP_VOLT_REG;
        mcp.u16MCPStatus &= ~MCP_VOLT_STABLE;
    }
}

void Read_Calib(void)
{
    uint8_t i;
    uint16_t *pCalibData;
    memset(&cal,0,sizeof(cal));
    pCalibData = &cal.u16Calib800mA;
    for(i=0;i<4;i++)
    {
        *pCalibData = Read_Flash(CAL_BASE_ADDR+i);
        pCalibData++;
    }
}

void MCP_Regulation_Init(void)
{
    
    memset((int8_t *)(&mcp),0x00,sizeof(mcp));
    mcp.u8Version = MAIN_VER|SUB_VER;
    Read_Calib();
    //verify if there is calibration value stored
    if((cal.u16Calib800mA > MIN_CALIB_800MA) &&
       (cal.u16Calib800mA < MAX_CALIB_800MA) &&
       (cal.u16Calib800mV > MIN_CALIB_800MV) &&
       (cal.u16Calib800mV < MAX_CALIB_800MV) &&
       (cal.u16Calib8V4 > MIN_CALIB_8V4) &&
       (cal.u16Calib8V4 < MAX_CALIB_8V4) ){
        mcp.u16MCPStatus |= MCP_CALIBED;
    }
    else
    {
        memset((int8_t *)(&cal),0,sizeof(cal));
        mcp.u16MCPStatus &= ~MCP_CALIBED;
#ifdef DEBUG_ON
        //below for debug only
        cal.u16Calib800mA = 224;
        cal.u16Calib800mV = 131;
        cal.u16Calib8V4 = 1378;
        cal.u16CalibZeroA = 276;   
        Write_Flash(CAL_BASE_ADDR, (uint8_t *)(&cal.u16Calib800mA));
        mcp.u16MCPStatus |= MCP_CALIBED;
#endif
    }
    //wait for input arrive at 12V
    mcp.u16MCPStatus |= MCP_INIT;
    Turn_Off_Output();
    //u16BattVoltLow = cal.u16Calib8V4>>1+cal.u16Calib800mV+cal.u16Calib800mV>>3; //5.2V = 8.4/2+0.8+0.2
    //u16BattVoltHigh = cal.u16Calib8V4 + cal.u16Calib800mV + cal.u16Calib800mV>>1;    //8.4+0.8+0.8/2 = 9.6V
}

void Status_Maintain(void)
{
    static uint8_t  u8ErrorCounter = 0;
    //static uint8_t  u8DetPowerLimitCounter = 0;
    static uint8_t  u8ErrorDetectCounter = 0;
    static uint8_t  u8BattDetCounter = 0;
    if(mcp.u16MCPStatus & MCP_ERROR){
        //if there is error
        Turn_Off_Output();
        //mcp.u16MCPStatus &= ~MCP_DETECT_ON;
        //mcp.u16MCPStatus &= ~MCP_LOAD_DET;
        if(mcp.u16ErrorStatus){
            //Check if the error status is released
            if((mcp.u16MCPStatus & (MCP_BATT_DET|MCP_LOAD_DET))
                && (adc.u16Therm > THERM_OPEN_ADC)
                && (adc.u16VBat < cal.u16Calib800mV) 
                //current is lower than 100mA
                && (adc.u16IOut <(cal.u16CalibZeroA+(cal.u16Calib800mA>>3)))){    
                if(u8BattDetCounter++ > 3){
                    u8BattDetCounter = 0;
                    mcp.u16MCPStatus &= ~(MCP_BATT_DET|MCP_LOAD_DET);
                    //battery removed, clear all error except INUVP
                    mcp.u16ErrorStatus &= INUVP;    //special
                }
            }
            else{
                u8BattDetCounter = 0;      
            }
            if(mcp.u16ErrorStatus & INUVP)
            {
                if(adc.u16Vin > INPUT_THRESHOLD)
                {
                    mcp.u16ErrorStatus &= ~INUVP;
                    VINIF = 0;
                    VINIE = 1;
                }
            }
            if(mcp.u16ErrorStatus & INOVP)
            {
                ;
            }
            if(mcp.u16ErrorStatus & (OUTOCP|OUTUVP|OUTOVP))
            {
                //battery is ejected < 800mV
                if(  (adc.u16Therm > THERM_OPEN_ADC)
                  && (adc.u16VBat < cal.u16Calib800mV) 
                //current is lower than 100mA
                  && (adc.u16IOut <(cal.u16CalibZeroA+(cal.u16Calib800mA>>3)))){
                    /*if(mcp.u16MCPStatus & MCP_DETECT_ON){
                        if(u8ErrorCounter > 200)
                        {
                            //short is not removed
                            //hold the status
                            u8ErrorCounter = 0;
                            return;
                        }
                        else
                        {
                            u8ErrorCounter ++;
                        }
                    }*/
                    if(u8ErrorCounter < 8){
                        u8ErrorCounter++;
                    }
                    else{
                        mcp.u16ErrorStatus &= ~OUTOCP;
                        mcp.u16ErrorStatus &= ~OUTUVP;
                        mcp.u16ErrorStatus &= ~OUTOVP;
                        u8ErrorCounter = 0;
                    }
                }
            }
            if(mcp.u16ErrorStatus & OUTOCP){
                if(adc.u16VBat > (cal.u16Calib8V4>>1)){
                    //Battery is valid;
                    if((adc.u16Therm < THERM_OPEN_ADC)&&(adc.u16Therm > THERM_SHORT_ADC)){
                        if(adc.u16IOut < (cal.u16CalibZeroA+ (cal.u16Calib800mA>>3)))
                        {
                            mcp.u16ErrorStatus &= ~OUTOCP;
                        }
                    }
                }
            }
            
            if(mcp.u16ErrorStatus & THERM_OPEN){
                if((adc.u16VBat < (cal.u16Calib800mV<<1))&&
                        (adc.u16Therm > THERM_OPEN_ADC)){
                    mcp.u16ErrorStatus &= ~THERM_OPEN;
                    //mcp.u16MCPStatus &= ~MCP_BATT_DET;
                }
                else
                {
                    if(mcp.u16MCPStatus & MCP_BATT_DET)
                    {
                        if((adc.u16Therm < THERM_OPEN_ADC)&&(adc.u16Therm > THERM_SHORT_ADC)){
                            mcp.u16ErrorStatus &= ~THERM_OPEN;
                        }
                    }
                }
            }
            if(mcp.u16ErrorStatus & THERM_SHORT){
                if(adc.u16Therm > THERM_OPEN_ADC){
                    mcp.u16ErrorStatus &= ~THERM_SHORT;
                    //mcp.u16MCPStatus &= ~MCP_BATT_DET;
                }
            }
            if(mcp.u16ErrorStatus & THERM_OPEN){
                if(!(mcp.u16MCPStatus & MCP_BATT_DET)){
                    mcp.u16ErrorStatus &= ~THERM_OPEN;
                    //mcp.u16MCPStatus &= ~MCP_BATT_DET;
                }
            }
        }
        else{
            //All Errors are cleared
            mcp.u16MCPStatus &= ~MCP_ERROR;
        }
    }//end of Error
    else{
        //if there is no error
        if( (mcp.u16MCPStatus & MCP_INIT) ||
            (mcp.u16MCPStatus & MCP_REMOTE) ||
            (!(mcp.u16MCPStatus & MCP_CALIBED)) ){
            //no behavior during initial
            //top level maintain in remote mode
            if((mcp.u16MCPStatus & MCP_INIT) && (adc.u16Vin> INPUT_THRESHOLD) && (VINIE == 0))
            {
                VINIF = 0;
                VINIE = 1;
                OCIF = 0;
                OCIE = 1;
                //mcp.u16MCPStatus &= ~MCP_INIT;
                /* update the zero current calibration value 
                if(adc.u16IOut != cal.u16CalibZeroA){
                    if((adc.u16IOut>(cal.u16CalibZeroA+1)) || (adc.u16IOut<(cal.u16CalibZeroA-1))){
                        cal.u16CalibZeroA = adc.u16IOut;
                    }
                }*/
            }
            return;
        }//end of if idle, init, remote
        else{
            //Check Thermistor short
            if(adc.u16Therm < THERM_SHORT_ADC){
                mcp.u16ErrorStatus |= THERM_SHORT;
                mcp.u16ErrorStatus |= MCP_ERROR;
                return;
            }
            //Check Current >3.2A
            if( (adc.u16IOut > ((cal.u16Calib800mA<<2)+cal.u16CalibZeroA))
                    || (OCIF == 1) ){
                OCIF = 0;
                if(u8ErrorDetectCounter > 10){
                    u8ErrorDetectCounter = 0;
                    mcp.u16ErrorStatus |= OUTOCP;
                    mcp.u16MCPStatus |= MCP_ERROR;
                }
                else{
                    u8ErrorDetectCounter ++;
                }
                return;
            }
            //Check input voltage
            if((adc.u16Vin) < (INPUT_THRESHOLD-30)){
                //Vin < 10.6V~
                if(u8ErrorDetectCounter > 10){
                    u8ErrorDetectCounter = 0;
                    mcp.u16ErrorStatus |= INUVP;
                    mcp.u16MCPStatus |= MCP_ERROR;
                    Turn_Off_Output();
                }
                else{
                    u8ErrorDetectCounter ++;
                }
                return;
            }
            if((adc.u16Vin) > INPUT_THRESHOLD+250){
                //Vin > 13V
                if(u8ErrorDetectCounter > 10){
                    u8ErrorDetectCounter = 0;
                    mcp.u16ErrorStatus |= INOVP;
                    mcp.u16MCPStatus |= MCP_ERROR;
                    Turn_Off_Output();
                }
                else{
                    u8ErrorDetectCounter ++;
                }
                return;
            }
            if(mcp.u16MCPStatus & MCP_BATT_DET)
            {
                //Battery installation detected
                //Check Thermistor open
                if(adc.u16Therm > THERM_OPEN_ADC)
                {
                    mcp.u16ErrorStatus |= THERM_OPEN;
                    mcp.u16MCPStatus |= MCP_ERROR;
                    return;
                }
                //Volt below error checking
                if((mcp.u16MCPStatus & (MCP_PWMOUT_ON)) && (adc.u16VBat < (cal.u16Calib800mV<<2)))
                {
                    //VBAT < 3.2V
                    if(u8ErrorDetectCounter > 20){
                        u8ErrorDetectCounter = 0;
                        mcp.u16ErrorStatus |= OUTUVP;
                        mcp.u16MCPStatus |= MCP_ERROR;
                    }
                    else{
                        u8ErrorDetectCounter++;
                    }
                    return;
                }
                //Volt over error checking
                if((adc.u16VBat > (cal.u16Calib8V4+ (cal.u16Calib800mV<<1))) &&(mcp.u8OVCC > 35))       //>9.6V
                {
                    //VBAT > 8.4+0.8 = 9.2V, line voltage drop should be 0.3V@2A
                    if(u8ErrorDetectCounter > 10){
                        u8ErrorDetectCounter = 0;
                        mcp.u16ErrorStatus |= OUTOVP;
                        mcp.u16MCPStatus |= MCP_ERROR;
                    }
                    else{
                        u8ErrorDetectCounter++;
                    }
                    return;
                }
                
                //no error detect, clear the error counter
                u8ErrorDetectCounter = 0;
            }//end of if battery detect
            else
            {
                if((adc.u16Therm < THERM_OPEN_ADC) && (adc.u16Therm > THERM_SHORT_ADC)){
                    //Thermistor detect
                    if(mcp.u16MCPStatus & MCP_LOAD_DET)
                    {
                        if((adc.u16VBat > (cal.u16Calib8V4 -(cal.u16Calib800mV<<2)+(cal.u16Calib800mV>>2)) ) && (adc.u16VBat <= (cal.u16Calib8V4 + cal.u16Calib800mV))){
                            //>5.4V batt detect 8.4-0.8*4+0.8/4 = 5.4V
                            if(u8BattDetCounter ++ > 20){
                                u8BattDetCounter = 0;
                                if(mcp.u16MCPStatus & MCP_PWMOUT_ON){
                                    if(adc.u16IOut>= (cal.u16CalibZeroA+(cal.u16Calib800mA>>3))){
                                        if(adc.u16VBat > (cal.u16Calib8V4 - (cal.u16Calib800mV<<1) - (cal.u16Calib800mV))){
                                            // > 6V batt
                                            Turn_Off_Output();
                                        }                                    
                                    }
                                    else{
                                        if(adc.u16VBat < (cal.u16Calib8V4-(cal.u16Calib800mV>>1))){
                                            if(mcp.u8OVCC < 35){
                                                Inc_Iout();
                                            }
                                            else{
                                                Turn_Off_Output();
                                            }
                                        }
                                        else{
                                            if(adc.u16VBat> (cal.u16Calib8V4-(cal.u16Calib800mV>>2))){
                                                Dec_Iout();
                                                if(u16BattDetTimer ++ > 20){
                                                    mcp.u16MCPStatus |= MCP_BATT_DET;
                                                    u16BattDetTimeoutCounter = 0;
                                                    u16BattDetTimer = 0;
                                                    mcp.u16MCPStatus &= ~MCP_LOAD_DET;
                                                }
                                            }
                                        }
                                    }
                                }
                                else{
                                    mcp.u16MCPStatus |= MCP_BATT_DET;
                                    u16BattDetTimeoutCounter = 0;
                                    u16BattDetTimer = 0;
                                    mcp.u16MCPStatus &= ~MCP_LOAD_DET;
                                }
                            }
                        }//end if vbat within 5.4 to 9.2V
                        else{
                            u8BattDetCounter = 0;
                            if(u16BattDetTimeoutCounter++ > 18740){
                                //more than 10 mins
                                u16BattDetTimeoutCounter = 18741;
                                mcp.u16ErrorStatus |= SLOW_TIME_OUT;
                                mcp.u16MCPStatus |= MCP_ERROR;
                                return;
                            }
                            if(adc.u16VBat > (cal.u16Calib8V4+cal.u16Calib800mV)){
                                if(mcp.u16MCPStatus & MCP_PWMOUT_ON){
                                    Dec_Iout();
                                }
                                else{
                                    mcp.u16ErrorStatus |= OUTOVP;
                                    mcp.u16MCPStatus |= MCP_ERROR;
                                }
                            }
                            else{
                                //vBAT < 4.2V, charge to unlock
                                if(mcp.u16MCPStatus & MCP_PWMOUT_ON){
                                    if(adc.u16IOut < ((cal.u16Calib800mA>>2)+cal.u16CalibZeroA)){
                                        if(mcp.u8OVCC < 35){
                                            Inc_Iout();
                                        }
                                        else{
                                            mcp.u16ErrorStatus |= OUTOCP;
                                            mcp.u16MCPStatus |= MCP_ERROR;
                                        }
                                    }
                                }
                                else{
                                    Turn_On_Output();
                                    mcp.u8OVCF = 0;
                                    mcp.u8OVCC = 10;
                                    OVCCON = mcp.u8OVCC;
                                    OVFCON = mcp.u8OVCF;
                                    //CHGSW_Off();
                                }
                            }
                        }
                    }//end of if LOAD detect
                    else{
                        if(u8BattDetCounter++ > 10){
                            mcp.u16MCPStatus |= MCP_LOAD_DET;
                            u8BattDetCounter = 0;
                        }
                    }
                }//end of thermistor detection
                else{
                    u16BattDetTimeoutCounter = 0;
                    u8BattDetCounter = 0;
                    u16BattDetTimer = 0;
                    mcp.u16MCPStatus &= ~MCP_LOAD_DET;
                    if(mcp.u16MCPStatus & MCP_PWMOUT_ON){
                        Turn_Off_Output();
                    }
                }
                /*
                if(mcp.u16MCPStatus & MCP_LOAD_DET)
                {
                    if(mcp.u16MCPStatus & MCP_DETECT_ON){
                        //Load detected with 12V supply
                        //verify VBAT if it is <8.4V or a sudden voltage drop
                        if(adc.u16VBat > (cal.u16Calib8V4 + 50))
                        {
                            mcp.u16MCPStatus &= ~MCP_LOAD_DET;
                            return;
                        }
                        else{
                            //Turn 12V detect supply off if it is on
                            Turn_Off_Output();
                            mcp.u16MCPStatus &= ~MCP_DETECT_ON;
                        }
                    }
                    else
                    {
                        //12V detect supply is off
                        //verify VBAT if battery is still exits
                        if( adc.u16VBat < cal.u16Calib800mV )
                        {
                            //VBAT < 4.2V, there is no battery or short situation
                            mcp.u16MCPStatus &= ~MCP_LOAD_DET;
                            return;
                        }
                        if(adc.u16Therm > THERM_SHORT_ADC)
                        {
                            if(adc.u16Therm < THERM_OPEN_ADC){
                                mcp.u16MCPStatus |= MCP_BATT_DET;
                                mcp.u16MCPStatus &= ~MCP_LOAD_DET;
                            }
                            else{
                                mcp.u16ErrorStatus |= THERM_OPEN;
                                mcp.u16MCPStatus |= MCP_ERROR;
                            }
                        }
                        else{
                            mcp.u16ErrorStatus |= THERM_SHORT;
                            mcp.u16MCPStatus |= MCP_ERROR;
                        }
                        return;
                    }    
                }
                else
                {
                    //LOAD is not Detected
                    if(mcp.u16MCPStatus & MCP_DETECT_ON){
                        if(adc.u16VBat < (cal.u16Calib8V4 +50)){
                            if(adc.u16VBat < (cal.u16Calib800mV<<2)){
                                //VBAT < 3.2V, short detect
                                mcp.u16ErrorStatus |= OUTOCP;
                                mcp.u16MCPStatus |= MCP_ERROR;
                            }
                            else{
                                //Load detect
                                mcp.u16MCPStatus |= MCP_LOAD_DET;
                            }
                                
                        }
                    }
                    else{
                        if(!(mcp.u16MCPStatus & MCP_PWMOUT_ON)){
                            //PWM OFF
                            if(adc.u16VBat > (cal.u16Calib800mV<<2)){
                                //voltage detect over input
                                //battery existing
                                mcp.u16MCPStatus |= MCP_LOAD_DET;
                            }
                            else
                            {
                                //Turn on PWM
                                Turn_On_Output();
                                CHGSW_Off();
                                u8DetPowerLimitCounter = 0;
                            }
                        }
                        else{
                            //PWM ON
                            if(adc.u16VBat < (cal.u16Calib8V4+(cal.u16Calib800mV>>3)))
                            {
                                
                                if(u8DetPowerLimitCounter++ > 100)
                                {
                                    //Short or low resistance load on output
                                    //mcp.u16MCPStatus |= MCP_DETECT_ON;
                                    mcp.u16ErrorStatus |= OUTOCP;
                                    mcp.u16MCPStatus |= MCP_ERROR;
                                    Turn_Off_Output();
                                    u8DetPowerLimitCounter = 0;
                                }
                                else
                                {
                                    Inc_Iout();
                                }
                            }
                            else
                            {
                                //12V detect supply over output
                                u8DetPowerLimitCounter = 0;
                                mcp.u16MCPStatus |= MCP_DETECT_ON;
                            }
                        }
                        
                    }
                }
                */
            }
        }
    }
}