#include "sdc.h"
#include "spi.h"

#include "lcd.h"
void sdc_init(){
  
  spi_init();

  SDC_CS = 1;
  for(char i=0; i<10; i++) spi_send(0xFF);
  
  //lcd_cmd(CLEAR);
  
  if(sdc_cmd(CMD0, 0)!=1){
    lcd_cmd(LINE2);
    lcd_str((char *)"CMD0 Error.");
    return;
  }
  
  lcd_cmd(0x80|8);
  if(sdc_cmd(CMD8, 0x1AA)!=1){//version1
    lcd_str((char *)"1");
  }else{
    lcd_str((char *)"2");
  }
  
  spi_send(0xFF);
  spi_send(0xFF);
  spi_send(0xFF);
  if(spi_send(0xFF)!=0xAA){//error
    lcd_cmd(LINE2);
    lcd_str((char *)"CMD8 Error.");
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
    lcd_str((char *)"CMD58 Error.");
    return;
  }
  if(spi_send(0xFF)&0b01000000){//sdhc
    lcd_cmd(0x80|10);
    lcd_str((char *)"SDHC");
  }
  spi_send(0xFF);
  spi_send(0xFF);
  spi_send(0xFF);
  
  if(sdc_cmd(CMD16, 512)){//error
    lcd_cmd(LINE2);
    lcd_str((char *)"CMD16 Error.");
    return;
  }
  
  SDC_CS = 1;
  
  lcd_cmd(LINE2);
  lcd_str((char *)"Success!");
}

char sdc_cmd(unsigned char cmd, unsigned long arg){
  SDC_CS = 0;
  spi_send(0xFF);
  
  short time=0;
  while(spi_send(0xFF)!=0xFF&&time<500){
      time++;
      __delay_ms(1);
  }
  if(500==time){
      lcd_cmd(CLEAR);
      lcd_str("TIME OUT");
      return 0;
  }
  
  spi_send(cmd|0x40);
  spi_send(arg>>24);
  spi_send(arg>>16);
  spi_send(arg>>8);
  spi_send(arg);
  spi_send( (cmd==CMD0)?0x95:(cmd==CMD8)?0x87:0xFF );

  char r;
  do{
  	r = spi_send(0xFF);
  }while(r & 0b10000000);

  return r;
}

