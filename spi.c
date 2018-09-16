#define SPI_SSPIF SSP2IF
#define SPI_SSPBUF SSP2BUF
#define SPI_SSPSTAT SSP2STAT
#define SPI_SSPCON1 SSP2CON1

void spi_init(){
  SPI_SSPSTAT = 0b00000000;//middle, idle 
  SPI_SSPCON1 = 0b00110010;//clock high, SPI Master, FOSC/64
  SPI_SSPIF = 0;
}

char spi_send(char dat){
  SPI_SSPBUF = dt;
  while (SPI_SSPIF == 0);
  SPI_SSPIF = 0;
  return SPI_SSPBUF;
}
