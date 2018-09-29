#include <xc.h>

#ifndef _XTAL_FREQ
  #define _XTAL_FREQ 32000000
#endif

#define SPI_SSPBUF SSP2BUF
#define SPI_SSPSTAT SSP2STAT
#define SPI_SSPSTATbits SSP2STATbits
#define SPI_SSPCON1 SSP2CON1
#define SPI_SSPCON1bits SSP2CON1bits
#define SPI_SSPADD SSP2ADD

void spi_init();
unsigned char spi_transfer(char dt);
