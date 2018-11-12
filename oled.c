#include "oled.h"
#include "i2c.h"

#define NUL '\0'
#define OLED_ADD 0x78
void cont(char isSingle, char isData) {
    char cont = 0b00000000;
    if(isSingle==1) cont|=0b10000000;
    if(isData==1) cont|=0b01000000;
    i2c_write(cont);
}
void oled_init(){
    
    i2c_init();
    
    i2c_start(OLED_ADD);
    
    cont(1,0);
    i2c_write(0xAE);//display off
    
    cont(0,0);
    i2c_write(0xA8); //multiplex ratio
    i2c_write(63); //16~63
    
    cont(0,0);
    i2c_write(0xD3); //offset
    i2c_write(0x00);
    
    cont(1,0);
    i2c_write(0x40);
    
    cont(1,0);
    i2c_write(0xA1);
    
    cont(1,0);
    i2c_write(0xC8);
    
    cont(0,0);
    i2c_write(0xDA);
    i2c_write(0x02);
    
    cont(0,0);
    i2c_write(0x81);//contrast
    i2c_write(0);//0~255
    
    cont(1,0);
    i2c_write(0xA4);
    
    cont(1,0);
    i2c_write(0xA6);
    
    cont(0,0);
    i2c_write(0xD5);
    i2c_write(0x80);
    
    cont(0,0);
    i2c_write(0xDA);
    i2c_write(0x12);
    
    cont(0,0);
    i2c_write(0x20); //Addressing mode
    i2c_write(0x00);
    
    cont(0,0);
    i2c_write(0x22); //page
    i2c_write(0);
    i2c_write(7);
    
    cont(0,0);
    i2c_write(0x21); //column
    i2c_write(0);
    i2c_write(127);
    
    cont(0,0);
    i2c_write(0x8D);//charge
    i2c_write(0x14);
    
    cont(1,0);
    i2c_write(0xAF);//display on
    
    i2c_stop();
}
void oled_clear(){
    i2c_start(OLED_ADD);
    
    cont(0,0);
    i2c_write(0x20); //Addressing mode
    i2c_write(0x00); //horizon
    
    cont(0,0);
    i2c_write(0x21); //column
    i2c_write(0);
    i2c_write(127);
    
    cont(0,0);
    i2c_write(0x22); //page
    i2c_write(0);
    i2c_write(7);
    
    i2c_stop();
    
    i2c_start(OLED_ADD);
    cont(0,1);
    for(unsigned short i=0; i<128*8; i++) i2c_write(0x00);
    i2c_stop();
}

void oled_addStr(char *str){
    for(char idx=0; str[idx]!=NUL; idx++){
        i2c_start(OLED_ADD);
        cont(0,1);
        for(char b=0; b<7; b++){
            unsigned char byte = eeprom_read(7*(str[idx]-',')+b);
            i2c_write(byte);
        }
        i2c_write(0x00);
        
        i2c_stop();
    }
}

void oled_str(char *str, char lineNum){//16文字まで
    i2c_start(OLED_ADD);
    
    cont(0,0);
    i2c_write(0x20); //Addressing mode
    i2c_write(0x10);
    
    cont(0,0);
    i2c_write(0x21); //column
    i2c_write(0);
    i2c_write(127);
    
    cont(0, 0);
    i2c_write(0x22); //page
    i2c_write(0);
    i2c_write(7);
    
    cont(1, 0);
    i2c_write(0xB0|lineNum);//page
    
    i2c_stop();
    
    oled_addStr(str);
}



unsigned short beBig(unsigned char c){
    unsigned short r=0;
    for(char i=0; i<8; i++){
        unsigned short t = c&(1U<<i);
        r |= t<<(i);
        r |= t<<(i+1);
    }
    return r;
}
void oled_bigStr(char *str, char lineNum){//８文字まで
    
    i2c_start(OLED_ADD);
    
    cont(0, 0);
    i2c_write(0x20); //Addressing mode
    i2c_write(0x01); //vertical
    
    cont(0, 0);
    i2c_write(0x21); //column
    i2c_write(0);
    i2c_write(127);
    
    cont(0, 0);
    i2c_write(0x22); //page
    i2c_write(lineNum);
    i2c_write(lineNum+1);
    
    //cont(1, 0);
    //i2c_write(0xB0|lineNum);//page
    
    i2c_stop();
    
    for(char idx=0; str[idx]!=NUL; idx++){
        i2c_start(OLED_ADD);
        cont(0, 1);
        for(char b=0; b<7; b++){
            unsigned char byte = eeprom_read(7*(str[idx]-',')+b);
            
            unsigned short s = beBig(byte);
            i2c_write(s&0xFF);
            i2c_write(s>>8);
            i2c_write(s&0xFF);
            i2c_write(s>>8);
        }
        i2c_write(0x00);
        i2c_write(0x00);
        i2c_write(0x00);
        i2c_write(0x00);
        
        i2c_stop();
    }
}
