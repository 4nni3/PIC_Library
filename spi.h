#include <xc.h>

#define SPI_SSPBUF SSP2BUF
#define SPI_SSPSTAT SSP2STAT
#define SPI_SSPSTATbits SSP2STATbits
#define SPI_SSPCON1 SSP2CON1
#define SPI_CS LATAbits.LATA5

void spi_init();
char spi_transfer(char dt);
