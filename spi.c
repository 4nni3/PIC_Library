#define SSPIF SSP1IF
#define SSPBUF SSP1BUF

void spi_init(){
}

char spi_send(char dat){
  SSPBUF = dt;
  while (SSPIF == 0);
  SSPIF = 0;
  return SSPBUF;
}
