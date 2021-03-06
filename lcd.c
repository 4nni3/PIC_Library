#include <stdio.h>
#include "lcd.h"
#include "i2c.h"

void lcd_init(){
    i2c_init();
    
    //__delay_ms(100);
    
    lcd_cmd(0x38);
    lcd_cmd(0x39);
    lcd_cmd(0x14);
    lcd_cmd(0x73);//contrast
    lcd_cmd(0x56);//contrast
    lcd_cmd(0x6C);
    lcd_cmd(0x38);
    lcd_cmd(CLEAR);
    lcd_cmd(0x0C);
}

void lcd_cmd(char c){
    i2c_start(LCD_ADD);
    i2c_write(0x00);
    i2c_write(c);
    i2c_stop();
    __delay_ms(20);
}

void lcd_dat(char d){
    i2c_start(LCD_ADD);
    i2c_write(0x40);
    i2c_write(d);
    i2c_stop();
}

void lcd_str(char *str){
    for(char i=0; str[i]!='\0'; i++){
        lcd_dat(str[i]);
    }
}

char c = 0;
void lcd_debug(char *b){
    for(char i=0; b[i]!='\0'; i++){
        if(c==16) lcd_cmd(LINE2);
        if(c==32){
            lcd_cmd(HOME);
            c=0;
        }
        lcd_dat(b[i]);
        c++;
    }
    if(c==16) lcd_cmd(LINE2);
    if(c==32){
        lcd_cmd(HOME);
        c=0;
    }
    lcd_dat(',');
    c++;
        
    
}