
#pragma config MCLRE = ON       //MCLR function enable
#pragma config CP = OFF         //Code Protection disable
#pragma config PWRTE = OFF      //Power up timer enable disable
#pragma config WDTE = OFF       //Watchdog timer enable disable
#pragma config WRT = OFF        //Flash program memory write protection OFF

#include "misc.h"

//Calibration storage
volatile const int16_t calibrate_data[4]@CAL_BASE_ADDR={0x0000,0x0000,0x0000,0x0000};// = {0x3fff,0x3fff,0x3fff,0x3fff};

//lite RTOS
uint8_t  u8SliceCounter,u8TimerSlotCounter;

//ADC sampling
uint8_t u8AdcChannelCounter = 0;
//u8AdcChannel[] shall be edited together with struct ADC_BUFFER, channels shall be aligned
uint8_t u8AdcChannel[] = {ADC_MUX_VIN, ADC_MUX_VBAT, ADC_MUX_IOUT, ADC_MUX_THERM};

ADC_BUFFER  adc;
uint16_t *pAdcData;

uint16_t u16CommTimeOut = 0;

CALIB_VALUE cal;

struct HW_Cal_Value{
    uint16_t    u16DiffGain;
    uint16_t    u16DiffOffset;
}HWCal;

void SYSTEM_Init(void)
{
    Regulator_Init();
    Cal_Init();
    IO_Init();
    I2C_Initialize();
    Timer0_Init();
    ADC_Init();
    return;
}

void IO_Init(void)
{
    //Port A initialize
    //Analog select for A/D input
    ANSELA = (GPIOA_THERM_PIN|GPIOA_NBATT_PIN);
    //Enable output only
    TRISGPA = 0xFF & ~(GPIOA_WDFEED_PIN|GPIOA_GPA4_PIN);
    //Weak pull-up disable on Output, Analog and SCL  ;
    WPUGPA = 0xff & ~(GPIOA_THERM_PIN|GPIOA_WDFEED_PIN|GPIOA_GPA4_PIN|GPIOA_SCL_PIN|GPIOA_NBATT_PIN);  
    //GPA is set to 0
    PORTGPA = 0x00;
    
    //Port B Initialize
    ANSELB = GPIOB_VBATT_PIN;  
    TRISGPB = 0xff & ~(GPIOB_CHGSW_PIN); 
    WPUGPB = 0xff & ~(GPIOB_CHGSW_PIN|GPIOB_VBATT_PIN|GPIOB_SDA_PIN);
    PORTGPB = 0x00;
    /*
    IOCA1 = 1;
    IOCF = 0;
    IOCE = 1;
    */
    return;
}

void Cal_Init(void)
{
    //Read calibration data and set into the appropriate register
    CALSEL = 1;
    PMADRH = 0x20; 
    PMADRL = 0x80; 
    RD = 1; 
    NOP(); 
    NOP(); 
    DOVCAL = PMDATH;    // Output voltage differential amp offset
    OSCCAL = PMDATL;    // Internal Oscillator calib
     
#ifdef MCP19118 //MCP19118/19
    CALSEL = 1; 
    PMADRL = 0x81; 
    RD = 1; 
    NOP(); 
    NOP(); 
    VROCAL = PMDATH;    // voltage offset calibration 
    BGRCAL = PMDATL;    // internal band gap calibration
    
    CALSEL = 1;
    PMADRL = 0x82;
    RD = 1; 
    NOP(); 
    NOP(); 
    TTACAL = PMDATH;    // over temperature shutdown threshold calib 
    ZROCAL = PMDATL;    // error amplifier offset voltage
    
    CALSEL = 1;
    PMADRL = 0x88; 
    RD = 1; 
    NOP(); 
    NOP(); 
    HWCal.u16DiffGain = PMDATH; //Available only for MCP19118/19
    HWCal.u16DiffGain <<= 8;
    HWCal.u16DiffGain |=PMDATL;
    
    CALSEL = 1;
    PMADRL = 0x89; 
    RD = 1; 
    NOP(); 
    NOP(); 
    HWCal.u16DiffOffset = PMDATH; //Available only for MCP19118/19
    HWCal.u16DiffOffset <<= 8;
    HWCal.u16DiffOffset |=PMDATL;
#else   //MCP19110/11     
    CALSEL = 1;
    PMADRL = 0x82;
    RD = 1; 
    NOP(); 
    NOP(); 
    VROCAL = PMDATH; 
    BGRCAL = PMDATL;
    
    CALSEL = 1; 
    PMADRL = 0x83; 
    RD = 1; 
    NOP(); 
    NOP(); 
    TTACAL = PMDATH; 
    ZROCAL = PMDATL;
#endif
    return;
}

void Timer0_Init(void)
{
    /* TMR0 1:4 prescale, 512 us per timer overflow */
    OPTION_REG = 0x01;      //bit7:0 Port GPx pull-ups enable
                            //bit6:0 Interrupt on falling edge of INT
                            //bit5:0 Internal instruction cycle clock    
                            //bit4:0 Increment on low-to-high on T0CKI pin
                            //bit3:0 Prescaler is assigned to the Timer0 module
                            //bit2 to 0 = 001 TMR0 rate = 1:4
    T0IF = 0;
    T0IE = 1;
    return;
}

void ADC_Init(void)
{
    /* ADC Initialization */
    pAdcData = (uint16_t *)&adc;
    u8AdcChannelCounter = 0;
    ADCON0 = u8AdcChannel[u8AdcChannelCounter];     /* Default input channel and ADON*/
    ADCON1 = 0x20; /* ADC  , FOSC/32*/
    ADON = 1;
    //ADIF = 0;
    //ADIE = 1;
    GO_nDONE =1;
    return;
}

void Timer0_ISR(void)
{
    uint8_t u8RTOS_Temp;
    //GIE = 0;
    WDFEED_Toggle();
    T0IF = 0;
    TMR0 += 8;  //balance of 1s 
    u8RTOS_Temp = u8SliceCounter++;
    u8TimerSlotCounter |= ((u8RTOS_Temp ^ u8SliceCounter) + 1);
    if(u16CommTimeOut++ > 5000){
        Turn_Off_Output();
        //I2C_Initialize();
    }
    else{
        
    }
    //GIE = 1;
    return;
}

void ADC_Pooling(void)
{
    uint16_t u16TempADC = 0;
    static uint16_t u16ADCx32 = 0;
    static uint16_t u16AdcCount = ADC_SAMPLE_RATE;
    
    if(u16AdcCount){
        u16AdcCount -- ;
        u16TempADC = ADRESH;
        u16TempADC <<= 8;
        u16TempADC |= ADRESL;
        u16ADCx32 += u16TempADC;
    }
    else{
        u16AdcCount = ADC_SAMPLE_RATE;
        //Verify if there is advanced after divided
        if((u16ADCx32 % SHIFT_MOD) < (SHIFT_MOD>>1)){
            *pAdcData = (u16ADCx32>>SHIFT_BITS);
        }
        else{
            *pAdcData = (u16ADCx32>>SHIFT_BITS);
            *pAdcData += 1;
        }
        u16ADCx32 = 0;
        u8AdcChannelCounter++;
        if(u8AdcChannelCounter < sizeof(u8AdcChannel)){
            pAdcData++;
        }
        else{
            u8AdcChannelCounter = 0;
            pAdcData = (uint16_t *)(&adc.u16Vin);
        }
        ADCON0 = u8AdcChannel[u8AdcChannelCounter];
        ADON = 1;
    }
    GO_nDONE =1;
    return;
}

/*
 * ADC_ISR(void)
 * Runtime 50us
 * Freq every 160us 

void ADC_ISR(void)
{
    uint16_t u16TempADC = 0;
    static uint16_t u16ADCx32 = 0;
    static uint16_t u16AdcCount = 0;
    
    PEIE = 0;
    ADIF = 0;
    if(u16AdcCount){
        u16AdcCount -- ;
        u16TempADC = ADRESH;
        u16TempADC <<= 8;
        u16TempADC |= ADRESL;
        u16ADCx32 +=u16TempADC;
    }
    else{
        u16AdcCount = ADC_SAMPLE_RATE;
        //Verify if there is advanced after divided
        if((u16ADCx32 % SHIFT_MOD) < (SHIFT_MOD>>1)){
            *pAdcData = (u16ADCx32>>SHIFT_BITS);
        }
        else{
            *pAdcData = (u16ADCx32>>SHIFT_BITS);
            *pAdcData += 1;
        }
        u16ADCx32 = 0;
        u8AdcChannelCounter++;
        if(u8AdcChannelCounter < sizeof(u8AdcChannel)){
            pAdcData++;
        }
        else{
            u8AdcChannelCounter = 0;
            pAdcData = &adc;
        }
        ADCON0 = u8AdcChannel[u8AdcChannelCounter];
        ADON = 1;
    }
    GO_nDONE = 1;
    PEIE = 1;
    return;
}
*/

/* This function write 8 bytes (4 flash words) at a time to flash.
 * The incoming address must be 8-byte aligned.
 */
void Write_Flash(uint16_t addr, uint8_t * Pdata) {
    uint8_t b = 0;
    //GIE = 0;
    if (addr < CAL_BASE_ADDR) return;

    while (b < 8) {
        PMADRH = addr >> 8;
        PMADRL = addr;
        PMDATL = *(Pdata+b);
        b++;
        PMDATH = *(Pdata+b);
        b++;
        CALSEL = 0;
        WREN = 1;
        PMCON2 = 0x55;
        PMCON2 = 0xAA;
        WR = 1;
        NOP();
        NOP();
        WREN = 0;
        addr++;
    }
    //GIE = 1;
    return;

}

uint16_t Read_Flash(uint16_t addr) {
    uint16_t a;
//GIE = 0;
    PMADRH = addr >> 8;
    PMADRL = addr;
    CALSEL = 0;
    RD = 1;
    NOP();
    NOP();
    a = PMDATH << 8;
    a |= PMDATL;
//GIE = 1;
    return a;
    
}

