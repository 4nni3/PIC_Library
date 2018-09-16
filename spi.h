#include <xc.h>

#define SPI_SSPIF SSP2IF
#define SPI_SSPBUF SSP2BUF
#define SPI_SSPSTAT SSP2STAT
#define SPI_SSPCON1 SSP2CON1

void spi_init();
char spi_send(char dat);
