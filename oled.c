#include "oled.h"
#include "i2c.h"

void oled_init(){
    
    i2c_start();
    
    oled_cont(1,0);
    i2c_write(0xAE);//display off
    
    oled_cont(0,0);
    i2c_write(0xA8); //multiplex ratio
    i2c_write(63); //16~63
    
    oled_cont(0,0);
    i2c_write(0xD3); //offset
    i2c_write(0x00);
    
    oled_cont(1,0);
    i2c_write(0x40);
    
    oled_cont(1,0);
    i2c_write(0xA1);
    
    oled_cont(1,0);
    i2c_write(0xC8);
    
    oled_cont(0,0);
    i2c_write(0xDA);
    i2c_write(0x02);
    
    oled_cont(0,0);
    i2c_write(0x81);//contrast
    i2c_write(0);//0~255
    
    oled_cont(1,0);
    i2c_write(0xA4);
    
    oled_cont(1,0);
    i2c_write(0xA6);
    
    oled_cont(0,0);
    i2c_write(0xD5);
    i2c_write(0x80);
    
    oled_cont(0,0);
    i2c_write(0xDA);
    i2c_write(0x12);
    
    oled_cont(0,0);
    i2c_write(0x20); //Addressing mode
    i2c_write(0x00);
    
    oled_cont(0,0);
    i2c_write(0x22); //page
    i2c_write(0);
    i2c_write(7);
    
    oled_cont(0,0);
    i2c_write(0x21); //column
    i2c_write(0);
    i2c_write(127);
    
    oled_cont(0,0);
    i2c_write(0x8D);//charge
    i2c_write(0x14);
    
    oled_cont(1,0);
    i2c_write(0xAF);//display on
    
    i2c_stop();
}
void oled_cont(char isSingle, char isData) {
    char cont = 0b00000000;
    if(isSingle==1) cont|=0b10000000;
    if(isData==1) cont|=0b01000000;
    i2c_write(cont);
}
void oled_clear(){
    i2c_start();
    
    oled_cont(0,0);
    i2c_write(0x20); //Addressing mode
    i2c_write(0x00);
    
    oled_cont(0,0);
    i2c_write(0x21); //column
    i2c_write(0);
    i2c_write(127);
    
    oled_cont(0,0);
    i2c_write(0x22); //page
    i2c_write(0);
    i2c_write(7);
    
    i2c_stop();
    
    i2c_start();
    oled_cont(0,1);
    for(unsigned short i=0; i<128*8; i++) i2c_write(0x00);
    i2c_stop();
}
void oled_str(char *str){
    i2c_start();
    
    oled_cont(0,0);
    i2c_write(0x20); //Addressing mode
    i2c_write(0x01);
    
    oled_cont(0,0);
    i2c_write(0x21); //column
    i2c_write(0);
    i2c_write(127);
    
    i2c_stop();
    
    for(char idx=0; str[idx]!=NUL; idx++){
        
        char line = idx/16; //current line
        
        if(idx%16==0){ //top char of line
            i2c_start();
            oled_cont(0,0);
            i2c_write(0x22); //page
            i2c_write(2*line);
            i2c_write(2*line+1);
            i2c_stop();
        }
        
        i2c_start();
        
        oled_cont(0,1);
        for(char b=0; b<7; b++){
            unsigned char byte = eeprom_read(7*(str[idx]-',')+b);
            i2c_write(byte<<4);
            i2c_write(byte>>4);
        }
        i2c_write(0x00);
        i2c_write(0x00);
        
        i2c_stop();
    }
}
