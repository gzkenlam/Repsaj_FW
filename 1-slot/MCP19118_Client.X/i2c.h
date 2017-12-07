/**

  @File Name
    i2c1.h

*/



#ifndef _I2C_H
#define _I2C_H

#include "misc.h"

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
                

/**
  I2C Slave Driver Status

  @Summary
    Defines the different status that the slave driver has
    detected over the i2c bus.

  @Description
    This defines the different status that the slave driver has
    detected over the i2c bus. The status is passed to the
    I2C_StatusCallback() callback function that is implemented by
    the user of the slave driver as a parameter to inform the user
    that there was a change in the status of the driver due to
    transactions on the i2c bus. User of the slave driver can use these
    to manage the read or write buffers.

 */

typedef enum
{
    I2C_SLAVE_WRITE_REQUEST,
    I2C_SLAVE_READ_REQUEST,
    I2C_SLAVE_WRITE_COMPLETED,
    I2C_SLAVE_READ_COMPLETED,
}I2C_SLAVE_DRIVER_STATUS;

/* command list of MCP19118 */    
#define NONE_CMD        0x00
#define INIT_CMD        0x01
#define PWM_ON          0x02
#define PWM_OFF         0x03
#define CHGSW_ON        0x04
#define CHGSW_OFF       0x05
#define TURN_ON         0x06
#define TURN_OFF        0x07
#define DEC_CURR        0x08
#define INC_CURR        0x09
#define VOLT_REG_ON     0x0a
#define VOLT_REG_OFF    0x0b
#define CURR_REG_ON     0x0c
#define CURR_REG_OFF    0x0d
#define CURR_REG_OUT    0x0e
#define VOLT_REG_OUT    0x0f
#define SET_ERROR       0x10
#define SET_STATUS      0x11
#define SET_CALIB       0x12
#define CLR_CALIB       0x13
#define CALIB_800MA     0x14
#define CALIB_800MV     0x15
#define CALIB_8V4       0x16
#define CALIB_ZEROA     0x17
#define WR_CALIB        0x18
#define RD_CALIB        0x19
#define SET_REMOTE      0x1a
#define CLR_REMOTE      0x1b
#define SET_THERM_ERR   0x1c
#define CLR_THERM_ERR   0x1d
#define SET_TIMER_ERR   0x1e
#define DEC_HIGH_CURR   0x28
#define INC_HIGH_CURR   0x29

typedef struct{
    uint8_t     u8SetCmd;
    uint16_t    u16SetValue1;
    uint16_t    u16SetValue2;
}SET_VALUE;

extern SET_VALUE  set;

#define I2C_SLAVE_DEFAULT_ADDRESS          0x15

//+3=2bytes of crc + 1 byte of count
#define TXBUF_SIZE  (30)//(sizeof(mcp)+sizeof(adc)+sizeof(cal)+3)         
#define RXBUF_SIZE  (8)//(sizeof(set)+3)          

#define PACK_END_1  0xCB
#define PACK_END_2  0xBC

/**
    @Summary
        Initializes and enables the i2c slave instance : 1

    @Description
        This routine initializes the i2c slave driver instance for : 1
        index, making it ready for clients to open and use it.

    @Preconditions
        None

    @Param
        None

    @Returns
        None

    @Example
        <code>
            // initialize the i2c slave driver
            I2C_Initialize();

        </code>
*/

void I2C_Initialize(void);

/**
   @Summary
        This function process the I2C interrupts generated by
        bus activity

    @Description
        This function calls a callback function with 1 of 4
        possible parameters.
            I2C_SLAVE_WRITE_REQUEST
            I2C_SLAVE_READ_REQUEST
            I2C_SLAVE_WRITE_COMPLETED
            I2C_SLAVE_READ_COMPLETED

        The callback function should contain application specific
        code to process I2C bus activity from the I2C master.
        A basic EEPROM emulator is provided as an example.
 */

void I2C_ISR ( void );

void I2C_Data_Handler(void);

/**
   @Summary
        This varible contains the last data written to the I2C slave
*/

extern volatile uint8_t    I2C_slaveWriteData;


#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif  // _I2C_H