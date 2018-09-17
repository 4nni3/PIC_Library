#include "spi.h"
#include "lcd.h"
#include <stdio.h>

void spi_init(){
  SPI_SSPSTAT = 0b00000000;//middle, idle 
  SPI_SSPCON1 = 0b00111010;//clock high, SPI Master, fosc
  SPI_SSPADD = 29;//300hz
  
  SPI_SSPIF = 0;
  PEIE = 1;
  GIE = 1;
}

char spi_send(char dt){
  short av = 0;
  SPI_SSPBUF = dt;
  while (SPI_SSPIF == 0) av++;
  SPI_SSPIF = 0;
  char d[6];
  sprintf(d, "%02X,%d", dt, av);
  lcd_cmd(0x80|2);
  lcd_str(d);
  
  return SPI_SSPBUF;
}
