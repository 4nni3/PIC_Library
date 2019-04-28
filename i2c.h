#include <xc.h>
#ifndef _XTAL_FREQ
  #define _XTAL_FREQ 32000000
#endif

#define I2C_SSPSTAT SSP1STAT //
#define I2C_SSPCON1 SSP1CON1 //
#define I2C_SSPCON2 SSP1CON2 //
#define I2C_SSPCON2bits SSP1CON2bits //
#define I2C_SSPCON3 SSP1CON3 //
#define I2C_SSPADD SSP1ADD //
#define I2C_SSPBUF SSP1BUF //

void i2c_init();
void i2c_wait();
void i2c_start(char add);
void i2c_stop();
void i2c_write(char data);
