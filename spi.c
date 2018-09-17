#include "spi.h"
#include "lcd.h"
#include <stdio.h>

void spi_init(){
  SPI_SSPSTAT = 0b00000000;//middle, idle 
  SPI_SSPCON1 = 0b00111010;//clock high, SPI Master, clk = fosc/(4*add+1)
  SPI_SSPADD = 29;//300kHz
  
  SPI_SSPIF = 0;
  SPI_SSPIE = 1;
  PEIE = 1;
  GIE = 1;
}

char spi_send(char dt){
  SPI_SSPBUF = dt;
  while(SPI_SSPIF == 0);
  SPI_SSPIF = 0;
  char r = SPI_SSPBUF;
  char b[4];
  sprintf(b, "S%02X", r);
  lcd_debug(b);
  return r;
}
