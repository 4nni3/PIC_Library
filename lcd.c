#include <xc.h>
#include "lcd.h"
#include "i2c.h"

void lcd_init(){
    
    i2c_init();
    
    __delay_ms(100);

    lcd_cmd(0x38);
    __delay_us(20);

    lcd_cmd(0x39);
    __delay_us(20);

    lcd_cmd(0x14);
    __delay_us(20);

    lcd_cmd(0x7A);
    __delay_us(20);

    lcd_cmd(0x50);
    __delay_us(20);

    lcd_cmd(0x6C);
    __delay_ms(20);

    lcd_cmd(0x38);
    __delay_ms(20);

    lcd_cmd(CLEAR);
    __delay_ms(20);

    lcd_cmd(0x0C);
    __delay_ms(20);
}

void lcd_cmd(char c){
    i2c_start(LCD_ADD);
    i2c_write(0x00);
    i2c_write(c);
    i2c_stop();
}

void lcd_dat(char d){
    i2c_start(LCD_ADD);
    i2c_write(0x40);
    i2c_write(d);
    i2c_stop();
}

void lcd_str(char *str){
    for(char i=0; i<16; i++){
        lcd_dat(str[i]);
    }
}
