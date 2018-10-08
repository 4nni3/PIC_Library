#include "i2c.h"

void i2c_init(){
    I2C_SSPSTAT = 0b10000000; //normal speed mode(100kHz)
    I2C_SSPCON1 = 0b00101000; //i2c master mode
    I2C_SSPCON3 = 0b00000000;
    I2C_SSPADD = (_XTAL_FREQ/4/100000)-1; //100kMHz clock = FOSC/((add+1)*4)
}

void i2c_wait(){
    while((I2C_SSPSTAT&0b00000100)||(I2C_SSPCON2&0b00011111));
}
void i2c_start(char add){
    i2c_wait();
    I2C_SSPCON2bits.SEN = 1;
    i2c_write(add);
}
void i2c_stop(){
    i2c_wait();
    I2C_SSPCON2bits.PEN = 1;
}
void i2c_write(char data) {
    i2c_wait();
    I2C_SSPBUF = data;
}
