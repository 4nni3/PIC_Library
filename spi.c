#include "spi.h"
#include "lcd.h"

void spi_init(){
    SPI_SSPSTAT = 0b00000000;//middle, idle 
    SPI_SSPCON1 = 0b00111010;//clock high, SPI Master, clk = fosc/4/(a+1)
    SPI_SSPADD = (_XTAL_FREQ/4/125000)-1;//125kHz;
}

unsigned char spi_transfer(char dt){
    char dummy = SPI_SSPBUF;
    SPI_SSPCON1bits.WCOL = 0;
    SPI_SSPBUF = dt;
    while(SPI_SSPSTATbits.BF==0);
    return (SPI_SSPBUF);
}
