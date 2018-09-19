#include <stdio.h>
#include "sdc.h"
#include "spi.h"

#include "lcd.h"
void sdc_init(){
  //__delay_ms(1000);
  
  char r;
  
  spi_init();

  SPI_CS = 1;
  for(char i=0; i<10; i++) spi_send(0xFF);
  
  r = sdc_cmd(CMD0, 0);
  if(r!=0x01){
    char b[24];
    sprintf(b, "Error CMD0 %02X", r);
    lcd_debug(b);
    return;
  }
  
  lcd_cmd(0x80|2);
  if(sdc_cmd(CMD8, 0x1AB)==0x05){//version1
    lcd_debug((char *)"1");
  }else{
    lcd_debug((char *)"2");
  }
  spi_transfer(0xFF);
  spi_transfer(0xFF);
  spi_transfer(0xFF);
  spi_transfer(0xFF);

  
  do{
    __delay_ms(1);
  	sdc_cmd(CMD55, 0);
  	r = sdc_cmd(ACMD41, 0x40000000);
  }while(r=0x01);

  if(sdc_cmd(CMD58, 0)){//error
    lcd_debug((char *)"CMD58 Error.");
    return;
  }
  lcd_cmd(0x80|4);
  if(spi_transfer(0xFF)&0b01000000){//sdhc
    lcd_debug((char *)"SDHC");
  }else{
    lcd_debug((char *)"SD");
  }
  spi_transfer(0xFF);
  spi_transfer(0xFF);
  spi_transfer(0xFF);
  
  if(sdc_cmd(CMD16, 512)){//error
    lcd_debug((char *)"CMD16 Error.");
    return;
  }
  
  lcd_debug((char *)"Success!");
}

char sdc_cmd(unsigned char cmd, unsigned long arg){
  spi_transfer(cmd|0x40);
  spi_transfer(arg>>24);
  spi_transfer(arg>>16);
  spi_transfer(arg>>8);
  spi_transfer(arg);
  spi_transfer( ((cmd==CMD0)||(cmd==CMD8))?0x95:0xFF );

  char r;
  do{
  	r = spi_transfer(0xFF);
  }while(r==0xFF);

  return r;
}
