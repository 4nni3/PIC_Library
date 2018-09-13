#include <xc.h>

#define SSPCON1 SSP1CON1
#define SSPCON2 SSP1CON2
#define SPPCON2bits SSP1CON2bits
#define SSPCON3 SSP1CON3
#define SSPADD SSP1ADD
#define SSPBUF SSP1BUF

void i2c_init();
void i2c_wait();
void i2c_start(char add);
void i2c_stop();
void i2c_write(char data);
