#include <xc.h>

#define _XTAL_FREQ 32000000

#define SDC_CS RA5

#define CMD0 0x40
#define CMD8 0x48
#define CMD55 0x77
#define CMD41 0x69
#define CMD16 0x50
#define CMD17 0x51
#define ACMD41 0x29

void sdc_init();
char sdc_cmd(char cmd, unsigned long arg, char crc=0xFF);
