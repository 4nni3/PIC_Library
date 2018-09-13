#include "i2c.h"

void i2c_init(){
    SSPSTAT = 0b10000000; //normal speed mode(100kHz)
    SSPCON1 = 0b00101000; //i2c master mode
    SSPCON3 = 0b00000000;
    SSPADD = 79; //100kMHz clock = FOSC/((add+1)*4)
}

void i2c_wait(){
    while((SSPSTAT&0b00000100)||(SSPCON2&0b00011111));
}
void i2c_start(char add){
    i2c_wait();
    SSPCON2bits.SEN = 1;
    i2c_write(add);
}
void i2c_stop(){
    i2c_wait();
    SSPCON2bits.PEN = 1;
}
void i2c_write(char data) {
    i2c_wait();
    SSPBUF = data;
}
