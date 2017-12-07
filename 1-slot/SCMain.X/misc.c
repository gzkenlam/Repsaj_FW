#include "misc.h"
#define _str(x)  #x
#define str(x)  _str(x)
#define RM_RESET_VECTOR 0x0600

//static uint16_t u16DelayMsec;
uint16_t u16I2CDelay;

extern uint16_t u16BusWait;

uint16_t u16CommTimeOut = 0;
uint8_t u8LEDOverridden = 0;
uint8_t u8LEDStatus;
uint8_t u8LEDDisplayCounter = 0;
uint8_t u8LEDPWM;

void Value_Init(void)
{
    u16CommTimeOut = 0;
    u8LEDOverridden = 0;
    u8LEDDisplayCounter = 0;
    u8LEDStatus = LED_OFF;
    //u16DelayMsec = 0;
    u16I2CDelay = 0;
    u16BusWait = 0;
}

void ResetDevice()
{
    asm ("pagesel " str(RM_RESET_VECTOR));
    asm ("goto  "  str(RM_RESET_VECTOR));   
   
}

void Delay_Msec(uint16_t Msec)
{   
    /*u16DelayMsec = Msec;
    while(u16DelayMsec);    */
    uint16_t i;
    uint16_t j;
    for(i=0;i<Msec;i++){
        for(j=0;j<785;j++);
    }
}

static uint8_t u8SliceCounter;
uint8_t u8TimerSlotCounter;

void Lite_RTOS_Timer(void)
{
    uint8_t u8RTOS_Temp;
    static uint8_t u8LEDSeq = 0;
    
    u8RTOS_Temp = u8SliceCounter++;
    u8TimerSlotCounter |= ((u8RTOS_Temp ^ u8SliceCounter) + 1);
    if(u16I2CDelay){
        u16I2CDelay--;
    }
    if(u16BusWait){
        u16BusWait -- ;
    }
    //TEST_Toggle();
    
    if(u16CommTimeOut ++ > 3000){
        u8LEDStatus=FBLINK_RED;
        //SYSTEM_Initialize();
        if(u16CommTimeOut < 4800){
            CLRWDT();
            TEST_Toggle();
        }
        else{
            u16CommTimeOut = 4800;
            ResetDevice();
        }
    }
    else{
        TEST_Toggle();
        CLRWDT();
    }
    
    if(u16SecCounter++ >999){
        u16SecCounter = 0;
        //1 Second;
        if((u16ChargerStatus & PRE_CHARGE)||(mcpdata.u16MCPStatus & MCP_LOAD_DET)){
            u16TotalChargeSec++;
            u16SlowTimeSec ++;
            if(u16SlowTimeSec > 3599){
                u16SlowTimeSec = 0;
                u8SlowTimeHr ++;
            }
        }
        if(u16ChargerStatus &(CC_CHARGE|CV_CHARGE)){
            u16TotalChargeSec++;
            u16FastTimeSec ++;
            if(u16FastTimeSec > 3599){
                u16FastTimeSec = 0;
                u8FastTimeHr ++;
            }
        }
    }
    u8LEDSeq ++;
    if(u8LEDSeq>63)
    {
        u8LEDSeq = 0;
        LED_Handle();
    }
}



void Set_LED(uint8_t u8led)
{
    if(u8LEDOverridden == 0){
        u8LEDStatus = u8led;
    }
    return;
}


void LED_Handle(void)       //Call every 32ms
{
    switch(u8LEDStatus)
    {
        case SOLID_RED:{
            PWM5_LoadDutyValue(LED_OFF_PWM);
            PWM6_LoadDutyValue(LED_RED_PWM);
            break;
        }
        case SOLID_GREEN:{
            PWM5_LoadDutyValue(LED_GRN_PWM);
            PWM6_LoadDutyValue(0);
            break;
        }
        case SOLID_AMBER:{
            PWM6_LoadDutyValue(LED_AMBER_RED_PWM);
            PWM5_LoadDutyValue(LED_AMBER_GRN_PWM);
            break;
        }
        case FBLINK_RED:{
            //250ms on, 250ms off
            PWM5_LoadDutyValue(LED_OFF_PWM);
            u8LEDDisplayCounter++;
            if(u8LEDDisplayCounter >7){
                u8LEDDisplayCounter =0;
            }
            if(u8LEDDisplayCounter < 4)
            {
                PWM6_LoadDutyValue(LED_RED_PWM);
            }
            else
            {
                PWM6_LoadDutyValue(LED_OFF_PWM);
            }
            break;
        }
        case BURST_AMBER:{
            u8LEDDisplayCounter++;
            if(u8LEDDisplayCounter >4){
                u8LEDDisplayCounter = 0; 
            }
            if(u8LEDDisplayCounter < 4)
            {
                PWM6_LoadDutyValue(LED_AMBER_RED_PWM);
                PWM5_LoadDutyValue(LED_AMBER_GRN_PWM);
            }
            else
            {
                PWM6_LoadDutyValue(LED_BURST_AMBER_GRN_PWM);
                PWM5_LoadDutyValue(LED_BURST_AMBER_RED_PWM);
            }
            break;
        }
        case BURST_GREEN:{
            PWM6_LoadDutyValue(LED_OFF_PWM);
            u8LEDDisplayCounter++;
            if(u8LEDDisplayCounter >4){
                u8LEDDisplayCounter = 0; 
            }
            if(u8LEDDisplayCounter < 4)
            {
                
                PWM5_LoadDutyValue(LED_GRN_PWM);
            }
            else
            {
                PWM5_LoadDutyValue(LED_BURST_GRN_PWM);
            }
            break;
        }
        case LED_OFF:{
            PWM6_LoadDutyValue(LED_OFF_PWM);
            PWM5_LoadDutyValue(LED_OFF_PWM);
            break;
        }
        default:{
            PWM6_LoadDutyValue(LED_OFF_PWM);
            PWM5_LoadDutyValue(LED_OFF_PWM);
            break;
        }       
    }
}

void MCP_I2C_Check(void)
{
    while(1){
        if( ( RC0_GetValue() ) && ( RC1_GetValue() ) )
        {
            EX_REST_SetHigh();
            return;
        }
        else{
            Set_LED(FBLINK_RED);
            EX_REST_SetLow();
            Delay_Msec(100);
            //EX_REST_SetHigh();
            //ResetDevice();
        }    
    }
}

/**
 End of File
*/