#include <xc.h>

#define SPI_SSPIF SSP2IF
#define SPI_SSPBUF SSP2BUF
#define SPI_SSPSTAT SSP2STAT
#define SPI_SSPSTATbits SSP2STATbits
#define SPI_SSPCON1 SSP2CON1
#define SPI_SSPADD SSP2ADD
#define SPI_SSPIE SSP2IE

void spi_init();
char spi_send(char dt);
