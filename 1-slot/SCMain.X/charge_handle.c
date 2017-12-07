#include "charge_handle.h"

int16_t s16MaxChargeCurr;
int16_t s16MaxChargeVolt;

uint8_t u8CalibStep = 0;
//static  uint16_t    u16SetCurr;
uint16_t u16SecCounter;
uint16_t    u16SlowTimeSec;
uint8_t u8SlowTimeHr;
uint16_t    u16FastTimeSec;
uint8_t  u8FastTimeHr;
uint16_t u16TotalChargeSec;
uint16_t u16ChargerStatus;
uint8_t u8MCPSendDone;

uint16_t u16CalibTimeoutCounter=0;


struct Cal_Value{
    uint16_t    u16Calib800mA;
    uint16_t    u16CalibZeroA;
    uint16_t    u16Calib800mV;
    uint16_t    u16Calib8V4;
}calval;




/*
 *  Send command to MCP19118, multi-call, call frequency shall be same
 *  than MCP Read/Write 
 *  Ret: 0, Sent successfully
 *  Ret: 1, Sending
 *  Ret: 2, Send Error after 3 times
 */
    
void Send_MCPCmd(uint8_t MCPCmd, uint16_t Val1, uint16_t Val2)
{
    if(u8MCPSendDone){
        mcpdata.u16MCPStatus &= ~MCP_GETCMD;
        cmd.u8SendCmd = MCPCmd;
        cmd.u16SetVal1 = Val1;
        cmd.u16SetVal2 = Val2;
        u8MCPSendDone = 0;
        //u16ChargerStatus &= 0xff00;
        //printf("bfs %x\r\n", u16ChargerStatus);
     }
}

/*
 *  ChargeTimer, shall be call very 2ms
 */
void ChargeTimer(void)
{
    if(u16ChargerStatus & (PRE_CHARGE|CC_CHARGE|CV_CHARGE|FULL_CHARGE)){
        if((u8SlowTimeHr >= battspec.u8SlowChargeTimeout) ||
           (u8FastTimeHr >= battspec.u8FastChargeTimeout)){
            //charge time out
            if(!(mcpdata.u16ErrorStatus & SLOW_TIME_OUT)){
                Send_MCPCmd(SET_TIMER_ERR,0,0);
            }
            else{
                //set err successfully;
                u16ChargerStatus &= ~(PRE_CHARGE|CC_CHARGE|CV_CHARGE|FULL_CHARGE);
                u8SlowTimeHr = 0;
                u8FastTimeHr = 0;
            }
        }
    }
}

uint16_t u16CalibTemp;
uint8_t u8MSG_SHOWED;
uint8_t u8CalibCounter = 0;
uint8_t u8DeJitterCounter=0;

void Calib_Err_Msg(void){
    if(!(u8MSG_SHOWED & 0x04)){
        puts("ERROR! Turn OFF/ON\r\n");
        u8MSG_SHOWED |= 0x04;
    }
}

/*
 *  User interactive input, load in UART 'Y' case
 */
void User_Yes_Interface(void)
{
    if(u16ChargerStatus & USER_INTERACT){
        u16ChargerStatus &= ~USER_INTERACT;
        puts("Y\r\n");
        if(u16ChargerStatus & WAIT_FOR_CALIB){
            u16ChargerStatus &= ~WAIT_FOR_CALIB;
            u16ChargerStatus |= CALIBING;
            u8CalibStep = 1;
        }
        else if(u16ChargerStatus & CALIBING){            
            u8CalibStep ++;
            u16CalibTimeoutCounter = 0;
        }
#ifdef DEBUG_MSG
        printf("CalibStep: %d\r\n",u8CalibStep);
#endif
    }
    else
    {
        puts("Don't hit Y\n\r");
    }
}

void User_No_Interface(void)
{
    if(u16ChargerStatus & USER_INTERACT){
        u16ChargerStatus &= ~USER_INTERACT;
        puts("No, set default value.\r\n");
        if(u16ChargerStatus & WAIT_FOR_CALIB){
            u16ChargerStatus &= ~WAIT_FOR_CALIB;
            u16ChargerStatus |= CALIBING;
            calval.u16CalibZeroA = mcpdata.u16IOut;
            u8CalibStep = 4;
        }
        else if(u16ChargerStatus & CALIBING){            
            switch(u8CalibStep){
                case 2:{//short
                    calval.u16CalibZeroA = mcpdata.u16IOut;
                    u8CalibStep = 4;
                    break;
                }
                case 5:{//8.4V
                    calval.u16Calib800mV = DEFAULT_CALIB_800mV;
                    calval.u16Calib8V4 = DEFAULT_CALIB_8V4;
                    u8CalibStep = 7;
                    break;
                }
                case 9:{//5Ohm
                    calval.u16Calib800mA = DEFAULT_CALIB_800mA;
                    u8CalibStep = 13;
                    break;
                }
                default:{
                    Calib_Err_Msg();
                    u8CalibStep = 0;
                    break;
                }
            }
        }
#ifdef DEBUG_MSG
        printf("CalibStep: %d\r\n",u8CalibStep);
#endif
    }
    else
    {
        puts("Don't hit N\n\r");
    }
}





void ADC_Calibration(void)
{
    static uint8_t u8ErrCount;
    switch (u8CalibStep){
        case 0:{
            if(mcpdata.u16MCPStatus & MCP_CALIBED){
                u8DeJitterCounter = 0;
                u8MSG_SHOWED = 0x00;
                return;
            }
            else{
                if(u8DeJitterCounter++ > 10)
                {
                    u8DeJitterCounter = 11;
                    u8ErrCount = 0;
                    u16ChargerStatus |= WAIT_FOR_CALIB;
                    if(mcpdata.u16MCPStatus & MCP_PWMOUT_ON){
                        Send_MCPCmd(TURN_OFF,0,0);
                    }
                    else{
                        if(mcpdata.u16MCPStatus & (~MCP_GETCMD)){
                            Send_MCPCmd(SET_STATUS, 0xffff, 0x00);
                        }
                    }
                    if(!(u8MSG_SHOWED & 0x01)){
                        puts("NOT CALIB, START?(Y/N)\r\n");
                        u16ChargerStatus |= USER_INTERACT;
                        u8MSG_SHOWED |= 0x01;
                        u16CalibTimeoutCounter = 0;
                        //Set_LED(BURST_AMBER);
                    }
                    else{
                        if(u16CalibTimeoutCounter++ > 60){
                            User_No_Interface();
                            u16CalibTimeoutCounter = 0;
                        }
                    }
                }
            }
            break;
        }
        case 1:{
            if(mcpdata.u16MCPStatus & MCP_CHGSW_ON){
                u8CalibStep++;
                u8ErrCount = 0;
            }
            else{
                Send_MCPCmd(CHGSW_ON,0,0);
                u8ErrCount ++;
                if(u8ErrCount > 5){
                    Calib_Err_Msg();
                    u8CalibStep = 0;
                    u8ErrCount = 0;
                }
            }
            break;
        }
        case 2:{
            if(!(u8MSG_SHOWED & 0x02)){
                puts("SHORT +/-,OK? (Y)\r\n");
                u16ChargerStatus |= USER_INTERACT;
                u8MSG_SHOWED |= 0x02;
                
            }
            else{
                if(u16CalibTimeoutCounter++ > 40){
                    User_No_Interface();
                    u16CalibTimeoutCounter = 0;
                }
            }
            u8CalibCounter = 0;
            break;
        }
        case 3:{
            if(mcpdata.u16VBat == 0){
                u8CalibCounter ++;
                if(u8CalibCounter > 5){
                    //zero stable
                    calval.u16CalibZeroA = mcpdata.u16IOut;
                    u8CalibCounter = 0;
                    u8CalibStep++;
                    u8ErrCount = 0;
                }                
            }
            else{
                u8CalibCounter = 0;
            }
            break;
        }
        case 4:{
            if(mcpdata.u16CalibZeroA == calval.u16CalibZeroA){
                u8CalibStep++;
                u8ErrCount =0;
            }
            else{
                Send_MCPCmd(CALIB_ZEROA,calval.u16CalibZeroA,0);
                u8ErrCount ++;
                if(u8ErrCount > 5){
                    Calib_Err_Msg();
                    u8CalibStep = 0;
                    u8ErrCount = 0;
                }                
            }
            break;
        }
        case 5:{
            if(!(u8MSG_SHOWED & 0x08)){
                puts("Put 8.4V on +/-,OK? (Y)\r\n");
                u16ChargerStatus |= USER_INTERACT;
                u8MSG_SHOWED |= 0x08;
                
            }
            else{
                if(u16CalibTimeoutCounter++ > 40){
                    User_No_Interface();
                    u16CalibTimeoutCounter = 0;
                }
            }
            u8CalibCounter = 0;
            break;
        }
        case 6:{
            if((mcpdata.u16VBat<1450)&&(mcpdata.u16VBat > 1200)){
                u8CalibCounter ++;
                if(u8CalibCounter > 5)
                {
                    calval.u16Calib8V4 = mcpdata.u16VBat;
                    calval.u16Calib800mV = (calval.u16Calib8V4<<1)/21;
                    u8CalibCounter = 0;
                    u8CalibStep++;
                }    
            }
            else{
                u8CalibCounter = 0;
            }
            break;
        }
        case 7:{
            if(mcpdata.u16Calib800mV == calval.u16Calib800mV){
                u8CalibStep++;
                u8ErrCount =0;
            }
            else{
                Send_MCPCmd(CALIB_800MV,calval.u16Calib800mV,0);
                u8ErrCount ++;
                if(u8ErrCount > 5){
                    Calib_Err_Msg();
                    u8CalibStep = 0;
                    u8ErrCount = 0;
                }
            }
            break;
        }
        case 8:{
            if(mcpdata.u16Calib8V4 == calval.u16Calib8V4){
                u8CalibStep++;
                u8ErrCount =0;
            }
            else{
                Send_MCPCmd(CALIB_8V4,calval.u16Calib8V4,0);
                u8ErrCount ++;
                if(u8ErrCount > 5){
                    Calib_Err_Msg();
                    u8CalibStep = 0;
                    u8ErrCount = 0;
                }
            }
            break;
        }
        case 9:{
            if(!(u8MSG_SHOWED & 0x10)){
                puts("Put 5R on +/-,OK? (Y)\r\n");
                u16ChargerStatus |= USER_INTERACT;
                u8MSG_SHOWED |= 0x10;
                //calculate 8V for current calibration in next step
                u16CalibTemp = calval.u16Calib8V4-(calval.u16Calib800mV>>1)+13;     
                
            }
            else{
                if(u16CalibTimeoutCounter++ > 40){
                    User_No_Interface();
                    u16CalibTimeoutCounter = 0;
                }
            }
            u8CalibCounter = 0;
            break;
        }
        case 10:{
            //u16Temp is from last step
            if(mcpdata.u16VoltGoal == u16CalibTemp){
                u8CalibStep++;
                u8ErrCount = 0;
                if(!(u8MSG_SHOWED & 0x20))
                {
                    puts("Wait...\r\n");
                    u8MSG_SHOWED |= 0x20;
                }
            }
            else{
                Send_MCPCmd(VOLT_REG_OUT,u16CalibTemp,0);
                u8ErrCount ++;
                if(u8ErrCount > 5){
                    Calib_Err_Msg();
                    u8CalibStep = 0;
                    u8ErrCount = 0;
                }
            }
            
            break;
        }
        case 11:{
            if(mcpdata.u16MCPStatus & MCP_VOLT_STABLE){
                u8CalibCounter ++;
                if(u8CalibCounter > 5)
                {
                    calval.u16Calib800mA = (mcpdata.u16IOut-calval.u16CalibZeroA)>>1;
                    u8CalibCounter = 0;
                    u8CalibStep++;
                }    
            }
            else{
                u8CalibCounter = 0;
            }
            break;
        }
        case 12:{
            if(mcpdata.u16MCPStatus & MCP_VOLT_STABLE){
                Send_MCPCmd(TURN_OFF,0,0);
                u8ErrCount ++;
                if(u8ErrCount > 5){
                    Calib_Err_Msg();
                    u8CalibStep = 0;
                    u8ErrCount = 0;
                }
            }
            else{
                u8CalibStep++;
                u8ErrCount = 0;
            }
            break;
        }
        case 13:{
            if(mcpdata.u16Calib800mA == calval.u16Calib800mA){
                u8CalibStep++;
                u8ErrCount =0;
            }
            else{
                Send_MCPCmd(CALIB_800MA,calval.u16Calib800mA,0);
                u8ErrCount ++;
                if(u8ErrCount > 5){
                    Calib_Err_Msg();
                    u8CalibStep = 0;
                    u8ErrCount = 0;
                }
            }
            
            break;
        }
        case 14:{
            Send_MCPCmd(WR_CALIB,0,0);
            u8CalibStep++;
            u8ErrCount ++;
            if(u8ErrCount > 5){
                Calib_Err_Msg();
                u8CalibStep = 0;
                u8ErrCount = 0;
            }
            break;
        }
        case 15:{
            if(mcpdata.u16MCPStatus & MCP_CALIBED){
                u8CalibStep++;
                u8ErrCount = 0;
            }
            else{                
                Send_MCPCmd(RD_CALIB,0,0);
                u8ErrCount ++;
                if(u8ErrCount > 2){
                    Calib_Err_Msg();
                    u8CalibStep --;
                    
                }
            }
            
            break;
        }
        case 16:{
            if(mcpdata.u16MCPStatus & MCP_CALIBED ){
                if(!(u8MSG_SHOWED & 0x40)){
                    u8MSG_SHOWED |= 0x40;
                    u8CalibStep = 0;
                    u16ChargerStatus &= ~CALIBING;
                    puts("Calib done!\r\n");
                    printf("Cal_8.4V = %d, Cal_800mV = %d, Cal_800mA = %d, Cal_0A = %d",
                            calval.u16Calib8V4,calval.u16Calib800mV,calval.u16Calib800mA,calval.u16CalibZeroA); 
                    u8MSG_SHOWED = 0x00;
                }
            }
            else{
                Calib_Err_Msg();
            }
            break;
        }
        default:break;
    }//end of switch case
}



void FG_Charge_Ctrl(void)
{
    static uint8_t u8ErrorCount = 0;

    if((battdata.u16TempK >= (battspec.u16ChargeLowTempK-20))            
        //some battery cannot be charged below 1C, adding 1.5C range for high low start temp
    && (battdata.u16TempK <= (battspec.u16ChargeHighTempK+15))){
        //Max volt and curr calulation basing on temperature of battery
        if(battdata.u16TempK <= battspec.u16SCLTempK1){
            s16MaxChargeCurr = battspec.u16SCLRate1;
            s16MaxChargeVolt = battspec.u16SCLVolt1;
        }
        else{
            if(battdata.u16TempK <= battspec.u16SCLTempK2){
                s16MaxChargeCurr = battspec.u16SCLRate2;
                s16MaxChargeVolt = battspec.u16SCLVolt2;
            }
            else{
               if(battdata.u16TempK <= battspec.u16SCLTempK3){
                   s16MaxChargeCurr = battspec.u16SCLRate2;
                   s16MaxChargeVolt = battspec.u16SCLVolt2;
               }
            }
        }
        //limitation of max voltage
        if(s16MaxChargeVolt > 8650){
            s16MaxChargeVolt = 8650;
        }
        
        //limitation of max current for device
        if(s16MaxChargeCurr > battspec.u16MaxCurr)
        {
            s16MaxChargeCurr = battspec.u16MaxCurr;
        }
        if(u8DebugOn){
            s16MaxChargeCurr = battspec.u16MaxCurr;
        }
        //Max Charge Current Limitation for temperature
        if(battdata.u16TempK >= 2730+375){  
            s16MaxChargeCurr = 1500;
            if(battdata.u16TempK >= 2730+405){
                //limit current to 900mA while thermistor > 41C
                s16MaxChargeCurr = 900;
                if(battdata.u16TempK >= 2730+425){
                    s16MaxChargeCurr = 500;
                }
            }
        }
        //limitation of max current for 12V input is too low
        if( mcpdata.u16Vin < INPUT_THRESHOLD ){
            if(s16MaxChargeCurr > 1000){
                s16MaxChargeCurr = 1000;
            }
            if(mcpdata.u16Vin < 580){
                Set_LED(LED_OFF);
                if(mcpdata.u16MCPStatus &(MCP_CHGSW_ON|MCP_PWMOUT_ON)){
                    Send_MCPCmd(TURN_OFF,0,0);
                    return;
                }
            }
        }        
        //insert this max current adjustment in manual debug mode
        
        //charge status maintain
        if(u16ChargerStatus & FULL_CHARGE){
            Set_LED(SOLID_GREEN);
            if(u8Unhealth){
                Set_LED(SOLID_RED);
            }
            if(mcpdata.u16MCPStatus &(MCP_CHGSW_ON|MCP_PWMOUT_ON)){
                Send_MCPCmd(TURN_OFF,0,0);
                return;
            }
            if(battdata.u16SOC < battspec.u8RechargeThreshold){
                u16ChargerStatus &= ~FULL_CHARGE;
                Set_LED(LED_OFF);
            }
        }
        
        if(u16ChargerStatus & CV_CHARGE){
            Set_LED(SOLID_AMBER);
            if((battdata.u16SOC >= 98) &&((battdata.s16Cur/2) <= (int16_t)battspec.u8ChargeTC)){
                //Shows green, but enter trickle charging
                Set_LED(SOLID_GREEN);
            }
            if(u8Unhealth){
                Set_LED(SOLID_RED);
            }
            if(battdata.s16Vol >= s16MaxChargeVolt){
                //decrease curr
                if((battdata.s16Cur <= (int16_t)battspec.u8ChargeTC)/*||(battdata.u16SOC == 100)*/){
                    u16ChargerStatus &= ~CV_CHARGE;
                    u16ChargerStatus |= FULL_CHARGE;
                }
                else{
                    //decrease current
                    Send_MCPCmd(DEC_CURR,0,0);
                }
            }
            else{
                //keep charge, not change
            }
            if((mcpdata.u16MCPStatus & MCP_PWMOUT_ON) == 0){
                u16ChargerStatus &= ~CV_CHARGE;
            }
        }//end of if(CV_CHARGE)
        if(u16ChargerStatus & CC_CHARGE){
            Set_LED(SOLID_AMBER);
            if(u8Unhealth){
                Set_LED(SOLID_RED);
            }
            if(battdata.s16Vol > s16MaxChargeVolt){
                //switching to CV charge mode
                u16ChargerStatus &= ~CC_CHARGE;
                u16ChargerStatus |= CV_CHARGE;

            }
            else{
                //constant curr charging 
                if(battdata.s16Cur > (s16MaxChargeCurr+2)){
                    if((battdata.s16Cur -s16MaxChargeCurr)>100){
                        Send_MCPCmd(DEC_HIGH_CURR,0,0);
                        //puts("Dec high\r");
                    }
                    else{
                        Send_MCPCmd(DEC_CURR,0,0);
                    }
                }
                else{
                    if(battdata.s16Cur < (s16MaxChargeCurr-2)){
                        if(((s16MaxChargeCurr-battdata.s16Cur)>200) && (battdata.u16SOC < 99)){
                            //increase current quickly as battery is not full and far from set current
                            Send_MCPCmd(INC_HIGH_CURR,0,0);
                            //puts("Inc high\r");                            
                        }
                        else{
                            Send_MCPCmd(INC_CURR,0,0);
                        }
                    }
                }
            }
            if((mcpdata.u16MCPStatus & MCP_PWMOUT_ON) == 0){
                u16ChargerStatus &= ~CC_CHARGE;
            }
        }//end of if(CC_CHARGE)
        if(u16ChargerStatus & PRE_CHARGE){
            Set_LED(SOLID_AMBER);
            if(u8Unhealth){
                Set_LED(SOLID_RED);
            }
            if(battdata.s16Vol > (int16_t)battspec.u16PrechargeThreshold){
                u16ChargerStatus &= ~PRE_CHARGE;
                u16ChargerStatus |= CC_CHARGE;

            }
            else{
                //constant 200mA curr charging 
                if(battdata.s16Cur >= 200){
                    Send_MCPCmd(DEC_CURR,0,0);    
                }
                else{
                    if(mcpdata.u16VBat < mcpdata.u16Calib8V4){
                        Send_MCPCmd(INC_CURR,0,0);
                    }
                    else{
                        if(mcpdata.u8OVCC < 30){
                            Send_MCPCmd(INC_CURR,0,0);
                        }
                    }
                }
            }
            if((mcpdata.u16MCPStatus & MCP_PWMOUT_ON) == 0){
                u16ChargerStatus &= ~PRE_CHARGE;
            }
        }//end of if(PRE_CHARGE)
        if((u16ChargerStatus &(PRE_CHARGE|CC_CHARGE|CV_CHARGE|FULL_CHARGE)) == 0)
        {
            //check if the battery is full charged
            if(battdata.u16Status & 0x0020){
                if(battdata.u16SOC > 92){
                    u16ChargerStatus |= FULL_CHARGE;
                }
            }
            //check if the battery is not allow to be charge, cycles > 600
            /* remove this feature by Devan recommendation
            if(battdata.u16Cycle >= 600)
            {
                if(mcpdata.u16MCPStatus & MCP_BATT_DET){
                    if(!(mcpdata.u16ErrorStatus & SLOW_TIME_OUT)){
                        Send_MCPCmd(SET_TIMER_ERR,0,0);
                    }      
                }
                return;
            }*/
            //check if battery within allow charging temperature
            if(battdata.u16TempK <= (battspec.u16ChargeHighTempK - 30)){
                //<=42C
                if((mcpdata.u16MCPStatus & (MCP_PWMOUT_ON|MCP_CHGSW_ON))==(MCP_PWMOUT_ON|MCP_CHGSW_ON)){
                    u16ChargerStatus |= PRE_CHARGE;
                    Set_LED(SOLID_AMBER);
                    u16SlowTimeSec = 0;
                    u16FastTimeSec = 0;
                    u8SlowTimeHr = 0;
                    u8FastTimeHr = 0;
                    u16TotalChargeSec = 0;
                    return;
                }
                Send_MCPCmd(TURN_ON,0,0);  
                
            }//end of if temperature < 42C
            else{
                //set Thermal over error
                if(mcpdata.u16MCPStatus & MCP_BATT_DET){
                    if(!(mcpdata.u16ErrorStatus & THERM_OVER)){
                        Send_MCPCmd(SET_THERM_ERR,0,0);               
                    }
                    return;
                }
            }
        }//end of if u16ChargerStatus = 0;
    }// end of if temperature within limitation
    else{
        //temperature exceeds limitation
        if(u8ErrorCount > 5){
            if(mcpdata.u16ErrorStatus & THERM_OVER){
            //set err successfully;
                u16ChargerStatus &= ~(PRE_CHARGE|CC_CHARGE|CV_CHARGE|FULL_CHARGE);
                u8ErrorCount = 0;
                return;
            }
            if(mcpdata.u16MCPStatus & MCP_BATT_DET){
                Send_MCPCmd(SET_THERM_ERR,0,0);  
            }
        }
        else{
            u8ErrorCount ++;
        }
        
    }
}

#define THERM_OPEN_ADC  0x0f00
#define THERM_SHORT_ADC 0x0064 
uint16_t u16BattDetTimeoutCounter;
uint8_t  u8BattDetCounter;
uint16_t u16BattDetTimer;
uint8_t u8ErrCounter;

void Remote_Batt_Det(void)
{
    if(mcpdata.u16ErrorStatus == 0){
        if(mcpdata.u16MCPStatus & MCP_BATT_DET){
            //Battery is detect, ERR checking
            if(mcpdata.u16Therm > THERM_OPEN_ADC){
                if(u8ErrCounter ++ > 2){
                    u8ErrCounter = 0;
                    if(!(mcpdata.u16ErrorStatus & THERM_OPEN))
                    {
                        Send_MCPCmd(SET_ERROR,0,THERM_OPEN);
                    }
                }
                return;
            }
            else if(mcpdata.u16Therm <THERM_SHORT_ADC){
                if(u8ErrCounter ++ > 2){
                    u8ErrCounter = 0;
                    if(!(mcpdata.u16ErrorStatus & THERM_SHORT))
                    {
                        Send_MCPCmd(SET_ERROR,0,THERM_SHORT);
                    }
                }
                return;
            }
            else{
                u8ErrCounter = 0;
            }
        }//end if of if Battery is detect
        else{
            if((mcpdata.u16Therm < THERM_OPEN_ADC) && (mcpdata.u16Therm > THERM_SHORT_ADC)){
                //Thermistor is detected
                if(mcpdata.u16MCPStatus & MCP_LOAD_DET){
                    if((mcpdata.u16VBat > (mcpdata.u16Calib8V4-(mcpdata.u16Calib800mV<<2)+(mcpdata.u16Calib800mV>>2))) && (mcpdata.u16VBat < (mcpdata.u16Calib8V4+(mcpdata.u16Calib800mV<<1)))){
                        //Battery voltage is >5.4V and <9.2V
                        if(u8BattDetCounter++ > 10){
                            u16BattDetTimeoutCounter = 0;
                            u8BattDetCounter = 0;
                            if(mcpdata.u16MCPStatus & MCP_PWMOUT_ON){
                                if((mcpdata.u16IOut > mcpdata.u16CalibZeroA) &&((mcpdata.u16IOut - mcpdata.u16CalibZeroA) > (mcpdata.u16Calib800mA>>3))){
                                    //if charge current > 100mA
                                    Send_MCPCmd(TURN_OFF,0,0);
                                }
                                else{
                                    if(mcpdata.u16VBat < (mcpdata.u16Calib8V4-(mcpdata.u16Calib800mV>>1))){
                                        //VBAT < 8V
                                        if(mcpdata.u8OVCC < 35){
                                            Send_MCPCmd(INC_CURR,0,0);
                                        }
                                    }
                                    else{
                                        if(mcpdata.u16VBat > (mcpdata.u16Calib8V4-(mcpdata.u16Calib800mV>>3))){
                                            //VBATT>8.3V
                                            if(u16BattDetTimer ++ > 10){
                                                //Force to set battery is detected
                                                u16BattDetTimeoutCounter = 0;
                                                u16BattDetTimer = 0;
                                                Send_MCPCmd(SET_STATUS,MCP_LOAD_DET,MCP_BATT_DET);
                                            }
                                            else
                                            {
                                                Send_MCPCmd(DEC_CURR,0,0);
                                            }
                                        }
                                    }
                                }
                            }
                            else{
                                //Battery is detected
                                u16BattDetTimeoutCounter = 0;
                                u16BattDetTimer = 0;
                                Send_MCPCmd(SET_STATUS,MCP_LOAD_DET,MCP_BATT_DET);
                            }
                        }                    
                    }//end if(5.4V<VBAT<8.4V))
                    else{
                        u8BattDetCounter = 0;
                        if(u16BattDetTimeoutCounter ++> 2400){                        
                            u16BattDetTimeoutCounter = 2400;
                            if(!(mcpdata.u16ErrorStatus & SLOW_TIME_OUT))
                            {
                                Send_MCPCmd(SET_TIMER_ERR,0,0);
                            }
                            return;
                        }
                        if(mcpdata.u16VBat > (mcpdata.u16Calib8V4+20)){
                            if(mcpdata.u16MCPStatus & MCP_PWMOUT_ON){
                                Send_MCPCmd(DEC_CURR,0,0);
                            }
                            else{
                                //BATT over voltage
                                if(!(mcpdata.u16ErrorStatus & OUTOVP))
                                {
                                    Send_MCPCmd(SET_ERROR,0,OUTOVP);
                                }
                                return;
                            }
                        }
                        else{
                            if(mcpdata.u16MCPStatus & MCP_PWMOUT_ON){
                                if(mcpdata.u16IOut< (mcpdata.u16CalibZeroA+(mcpdata.u16Calib800mA>>2))){
                                    if(mcpdata.u8OVCC < 35){                         
                                        Send_MCPCmd(INC_CURR,0,0);
                                    }
                                    else{
                                        if(!(mcpdata.u16ErrorStatus & OUTOCP)){
                                            Send_MCPCmd(SET_ERROR,0,OUTOCP);
                                        }
                                        return;
                                    }
                                }
                            }
                            else{
                                Send_MCPCmd(TURN_ON,0,0);
                            }
                        }
                    }
                }
                else{
                    Send_MCPCmd(SET_STATUS,0,MCP_LOAD_DET);
                }
            }//end of if(short<THERM<open)
            else{
                u16BattDetTimer = 0;
                u16BattDetTimeoutCounter = 0;
                if(mcpdata.u16MCPStatus & MCP_LOAD_DET){
                    Send_MCPCmd(SET_STATUS,MCP_LOAD_DET,0);
                }
            }
        }
    }
}

void Charger_Handle(void)
{
    static uint8_t u8LEDCounter = 0;
    static uint16_t u16OldErrorCode;        //for debug
    if(mcpdata.u16MCPStatus & MCP_ERROR){
        u16ChargerStatus &= ~(PRE_CHARGE|CC_CHARGE|CV_CHARGE|FULL_CHARGE);
        if(u8LEDCounter++ > 4){
            u8LEDCounter =5;
            Set_LED(FBLINK_RED);
        }
        else{
            Set_LED(LED_OFF);
        }
#ifdef DEBUG_MSG
        if(u16OldErrorCode != mcpdata.u16ErrorStatus){
            printf("ErrorCode: %x\r\n", mcpdata.u16ErrorStatus);    
            u16OldErrorCode = mcpdata.u16ErrorStatus;
        }
#endif
        if(mcpdata.u16ErrorStatus & THERM_OVER){
            //this error handle by high layer PIC16F18326
            if((battdata.u16TempK >= battspec.u16ChargeLowTempK) 
                && (battdata.u16TempK <= (battspec.u16ChargeHighTempK - 30))){
                    //temp within 0~42
                Send_MCPCmd(CLR_THERM_ERR,0,0);
                
            }
            /*if(battdata.u16Status == OTHER_BATT){
                //no fuel gauge info, not support
                
                if(mcpdata.u16MCPStatus & MCP_CALIBED){
                    if((mcpdata.u16Therm > NTC_HOT_START_ADC) && (mcpdata.u16Therm< NTC_COOL_START_ADC)){
                        u8Temp = Send_MCPCmd(CLR_THERM_ERR,0,0);
                        if(u8Temp == 0){
                            //clear err successfully
                        }
                        else if(u8Temp == 2){
                            Set_LED(FBLINK_RED);             
                        }
                    }
                }
                else{
                    Set_LED(FBLINK_RED);
                }
                
            }*/
        }
        return;
    }//end of if ERROR    
#ifdef  DEBUG_MSG
    u16OldErrorCode = 0;
#endif
    if(mcpdata.u16MCPStatus & MCP_INIT){
        if(mcpdata.u16Vin > INPUT_THRESHOLD){
            /* switch remote mode on 
            if(!(mcpdata.u16MCPStatus & MCP_REMOTE))
            {
                Send_MCPCmd(SET_STATUS,0,MCP_REMOTE);
                return;
            }*/
            u8LEDCounter++;
            if(u8LEDCounter <5){
                Set_LED(SOLID_AMBER);
                //puts("AMBER\r\n");
            }
            else if(u8LEDCounter <10){
                Set_LED(SOLID_GREEN);
                //puts("RED\r\n");
            }
            else if(u8LEDCounter <15){
                Set_LED(SOLID_RED);
                //puts("GREEN\r\n");
            }
            else if(u8LEDCounter >14){
                u8LEDCounter = 15;
                
                if(mcpdata.u16MCPStatus & MCP_INIT){
                    //verify zero current of calibration
                    if((mcpdata.u16IOut<=(mcpdata.u16CalibZeroA+1)) && (mcpdata.u16IOut>=(mcpdata.u16CalibZeroA-1))){
                        Send_MCPCmd(INIT_CMD,0,0);
                    }
                    else{
                        Send_MCPCmd(CALIB_ZEROA,mcpdata.u16IOut,0);
                    }
                }
                else{
                    u8LEDCounter = 0;
                    Set_LED(LED_OFF);
                }
                
            }
        }
        return;
    }
    if(((mcpdata.u16MCPStatus & MCP_CALIBED)/*||(mcpdata.u16MCPStatus & MCP_REMOTE)*/)
        &&(!(u16ChargerStatus & CALIBING))    ){
        //calibrated or remote mode should start charge control
        u8DeJitterCounter = 0;
        u8LEDCounter = 0;
        u16ChargerStatus &= ~(WAIT_FOR_CALIB|USER_INTERACT);
        if( battdata.u8Status ){
            //battery is detected
            if(battdata.u8Status == OTHER_BATT){
                Set_LED(SOLID_RED);
                if(mcpdata.u16MCPStatus &(MCP_CHGSW_ON|MCP_PWMOUT_ON)){
                    Send_MCPCmd(TURN_OFF,0,0);
                   
                }
                 /* batt without FG is not support
                if(!(mcpdata.u16MCPStatus & MCP_CALIBED)){
                    //remote mode cannot charge other batt without calibration
                    puts("REMOTE MODE not work\r\n");
                    return;
                }
                else{
                    //calibrated, call ADC_Charge_Control;
                    ADC_Charge_Ctrl();
                }
                */                
            }
            else{
                //battery with fuel gauge, call FG_Charge_Control;
                if((battdata.u16Status & 0x0007) == 0){
                    //no error code from battery fuel gauge status;
                    FG_Charge_Ctrl();
                }
                else{
                    //Set_LED(FBLINK_RED);
                    if(mcpdata.u16MCPStatus &(MCP_CHGSW_ON|MCP_PWMOUT_ON)){
                        Send_MCPCmd(TURN_OFF,0,0);   
                    }
                }
            }
            ChargeTimer();
            if(mcpdata.u16MCPStatus & MCP_REMOTE)
            {
                //call remote battery detect feature here
                Remote_Batt_Det();
            }
        }//end of if(battdata.u8Status > 0) battery type detected
        else
        {
            //no battery detected
            u16ChargerStatus &= ~(PRE_CHARGE|CC_CHARGE|CV_CHARGE|FULL_CHARGE);
            //Set_LED(LED_OFF);
            if(mcpdata.u16MCPStatus & MCP_LOAD_DET){
                //NTC detected
                Set_LED(SOLID_AMBER);
                if(u8SlowTimeHr >= 1){
                    //awake time out
                    if(!(mcpdata.u16ErrorStatus & SLOW_TIME_OUT)){
                        Send_MCPCmd(SET_TIMER_ERR,0,0);
                    }
                }
            }
            else{
                if(mcpdata.u16MCPStatus &(MCP_CHGSW_ON|MCP_PWMOUT_ON)){
                    Send_MCPCmd(TURN_OFF,0,0);
                }
            }
            if((mcpdata.u16MCPStatus & (MCP_LOAD_DET|MCP_BATT_DET|MCP_ERROR))==0){
                Set_LED(LED_OFF);
                u16SlowTimeSec = 0;
                u8SlowTimeHr = 0;
            }
            if(mcpdata.u16MCPStatus & MCP_REMOTE)
            {
                //call remote battery detect feature here
                Remote_Batt_Det();
            }
        }
    }
    else{
        //force to calibration mode
        ADC_Calibration();
    }
}

/* 
void ADC_Charge_Ctrl(void)
{
    uint8_t u8Temp;
    if( (mcpdata.u16Therm > NTC_HOT_CHARGE_ADC)
    && (mcpdata.u16Therm < NTC_COOL_CHARGE_ADC)){
        if(u16ChargerStatus & FULL_CHARGE){
            Set_LED(SOLID_RED);
            if(mcpdata.u16MCPStatus & (MCP_PWMOUT_ON|MCP_CHGSW_ON)){
                u8Temp = Send_MCPCmd(TURN_OFF,0,0);
            }
            if(mcpdata.u16VBat < (mcpdata.u16Calib8V4-(mcpdata.u16Calib800mV>>2))){
                u16ChargerStatus &= ~FULL_CHARGE;
                Set_LED(LED_OFF);
            }
        }//end of if full charge
        if(u16ChargerStatus & CV_CHARGE){
            Set_LED(SOLID_RED);
            if(mcpdata.u16VBat > battspec.u16SCLVolt1+10){
                if(mcpdata.u16IOut < battspec.u8ChargeTC){
                    u16ChargerStatus &= ~CV_CHARGE;
                    u16ChargerStatus |= FULL_CHARGE;
                }
                else{
                    //decrease current
                    u8Temp = Send_MCPCmd(DEC_CURR,0,0);
                    if(u8Temp == 0){
                        //decrease done;
                    }
                    
                }
            }
            else{
                //keep charge
            }            
        }//end of if CV charge
        if(u16ChargerStatus & CC_CHARGE){
            Set_LED(SOLID_RED);
            if(mcpdata.u16VBat > battspec.u16SCLVolt1+15){
                u16ChargerStatus &= ~CC_CHARGE;
                u16ChargerStatus |= CV_CHARGE;
            }
            else{
                //constant current at 800mA
                if(mcpdata.u16IOut > battspec.u16MaxCurr){
                    //decrease current
                    u8Temp = Send_MCPCmd(DEC_CURR,0,0);
                    if(u8Temp == 0){
                        //decrease done;
                    }
                    
                }
                else{
                    //increase current
                    u8Temp = Send_MCPCmd(INC_CURR,0,0);
                    if(u8Temp == 0){
                        //increase done;
                    }
                    
                }
            }            
        }//end of if CC charge
        if(u16ChargerStatus & PRE_CHARGE){
            Set_LED(SOLID_RED);
            if(mcpdata.u16VBat > battspec.u16PrechargeThreshold){
                //Vbat > 5.6 V
                u16ChargerStatus &= ~PRE_CHARGE;
                u16ChargerStatus |= CC_CHARGE;
            }
            else{
                //maintain pre charge current at 200mA
                if(mcpdata.u16IOut > ((mcpdata.u16Calib800mA>>2)+mcpdata.u16CalibZeroA)){
                    //decrease current
                    u8Temp = Send_MCPCmd(DEC_CURR,0,0);
                    if(u8Temp == 0){
                        //decrease done;
                    }
                    
                }
                else{
                    //increase current
                    u8Temp = Send_MCPCmd(INC_CURR,0,0);
                    if(u8Temp == 0){
                        //increase done;
                    }
                    
                }
            }
        }//end of if Pre charge
        if((u16ChargerStatus &(PRE_CHARGE|CC_CHARGE|CV_CHARGE|FULL_CHARGE)) == 0){
            //Set_LED(LED_OFF);
            if((mcpdata.u16Therm > NTC_HOT_START_ADC)
            && (mcpdata.u16Therm < NTC_COOL_START_ADC)){
                u8Temp = Send_MCPCmd(TURN_ON,0,0);
                if((mcpdata.u16MCPStatus & (MCP_PWMOUT_ON|MCP_CHGSW_ON))==(MCP_PWMOUT_ON|MCP_CHGSW_ON)){
                    u16ChargerStatus |= PRE_CHARGE;
                    u16SlowTimeSec = 0;
                    u16FastTimeSec = 0;
                    u8SlowTimeHr = 0;
                    u8FastTimeHr = 0;
                    u16TotalChargeSec = 0;
                }
            }
        }//end if no charge start
    }//end of if temperature within limitation
    else{
        //temperature exceeds limitation
        u8Temp = Send_MCPCmd(SET_THERM_ERR,0,0);
        if(mcpdata.u16ErrorStatus & THERM_OVER){
            //set err successfully;
            u16ChargerStatus &= ~(PRE_CHARGE|CC_CHARGE|CV_CHARGE|FULL_CHARGE);
        }
        
    }
}*/

/**
 End of File
*/