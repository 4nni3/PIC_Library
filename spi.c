#include "spi.h"
#include "lcd.h"
#include <stdio.h>

void spi_init(){
  SPI_SSPSTAT = 0b00000000;//middle, idle 
  SPI_SSPCON1 = 0b00110010;//clock high, SPI Master, clk = fosc/64
  SPI_CS = 1;
}

char spi_transfer(char dt){
  SPI_CS = 0;
  SPI_SSPBUF = dt;
  while(SPI_SSPSTATbits.BF == 0);
  char r = SPI_SSPBUF;
  SPI_CS = 1;
  return r;
}
