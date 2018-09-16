#include "sdc.h"
#include "spi.h"

#include "lcd.h"

#define CMD0 0x40
#define CMD8 0x48
#define CMD55 0x77
#define CMD41 0x69
#define CMD16 0x50
#define CMD17 0x51
#define ACMD41 0x29

#define SDC_CS RA5

void sdc_init(){

  sdc_init();

  SDC_CS = HIGH;
  for(char i=0; i<10; i++) spi_send(0xFF);
  SDC_CS = LOW;
  spi_send(0xFF);

  sdc_cmd(CMD0, 0, 0x95);

  if(sdc_cmd(CMD8, 0x1AA, 0x87)!=1){//version1
    lcd_str("ver1");
  }else{
    lcd_str("ver2");
  }
  spi_send(0xFF);
  spi_send(0xFF);
  spi_send(0xFF);
  if(spi_send(0xFF)!=0xAA){//error
    lcd_cmd(LINE2);
    lcd_str("CMD8 Error.");
    return;
  }

  char r;
  do{
  	sdc_cmd(CMD55, 0);
  	r = sdc_cmd(ACMD41, 0x40000000);
  	__delay_ms(1);
  }while(r!=0);

  if(sdc_cmd(CMD58, 0)){//error
    lcd_cmd(LINE2);
    lcd_str("CMD58 Error.");
    return;
  }
  if(spi_send(0xFF)&0b01000000){//sdhc
    lcd_cmd(0x80|6);
    lcd_str("sdhc");
  }
  spi_send(0xFF);
  spi_send(0xFF);
  spi_send(0xFF);
  
  if(sdc_cmd(CMD16, 512)){//error
    lcd_cmd(LINE2);
    lcd_str("CMD16 Error.");
    return;
  }

  SDC_CS = HIGH;

  lcd_cmd(LINE2);
  lcd_str("Success!");
}

char sdc_cmd(char cmd, unsigned long arg, char crc=0xFF){
  while(spi_send(0xFF)!=0);

  spi_send(cmd);
  spi_send(arg>>24);
  spi_send(arg>>16);
  spi_send(arg>>8);
  spi_send(arg);
  spi_send(crc);

  char r;
  do{
  	r = spi_send(0xFF);
  }while(r & 0b10000000);

  return r;
}

