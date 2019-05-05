

#include "soft_i2c.h"

void i2c_init(){
    SCL(1);
    SDA(1);
    SCL_OUT();
    SDA_OUT();
}

void i2c_start(void){
    SDA(1);
    WAIT();
    SCL(1);
    WAIT();
    SDA(0);
    WAIT();
    SCL(0);
    WAIT();
}

void i2c_stop(void){
    SDA(0);
    WAIT();
    SCL(1);
    WAIT();
    SDA(1);
}

char i2c_write(unsigned char wdata){
    for(char i=0; i<8; i++){
        if(wdata & 0x80) {
            SDA(1);
        }else{
            SDA(0);
        }
        WAIT();
        SCL(1);
        WAIT();
        SCL(0);
        WAIT();
        wdata<<=1;
    }
    
    SDA_IN();
    WAIT();
    
    SCL(1);
    WAIT();
    char ack = SDA_PIN;
    WAIT();
    SCL(0);
    WAIT();
    SDA(1);
    SDA_OUT();
    WAIT();
    return ack;
}
unsigned char i2c_read(char ack){
    SDA_IN();
    
    unsigned char rtn = 0x00;
    for(char i=0; i<8; i++){
    	SCL(1);
        NOP();
        rtn = (rtn<<1)|SDA_PIN;
        SCL(0);
    }
    
    SDA_OUT();
    
    SDA(ack);
    NOP();
    SCL(1);
    WAIT();
    WAIT();
    SCL(0);
    WAIT();
    SDA(1);
    WAIT();
    
    return rtn;
}


