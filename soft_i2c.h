#include <xc.h>

#define SDA_PIN  RA2
#define SCL_PIN  RA4
#define SDA_TRIS TRISA2
#define SCL_TRIS TRISA4

#define SDA_IN()    SDA_TRIS = 1
#define SDA_OUT()   SDA_TRIS = 0
#define SCL_IN()    SCL_TRIS = 1
#define SCL_OUT()   SCL_TRIS = 0

#define SCL(i)      SCL_PIN = i
#define SDA(i)      SDA_PIN = i

#define WAIT()      {NOP();NOP();NOP();NOP();NOP();}

void i2c_init();
void i2c_wait();
void i2c_start();
void i2c_stop();
char i2c_write(unsigned char wdata);
unsigned char i2c_read(char ack);
