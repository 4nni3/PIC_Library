#include <xc.h>

#ifndef _XTAL_FREQ
  #define _XTAL_FREQ 32000000//
#endif

#define CMD0   0x00
#define CMD1   0x01
#define CMD8   0x08
#define CMD12  0x0C
#define CMD13  0x0D
#define CMD16  0x10
#define CMD17  0x11
#define CMD24  0x18
#define ACMD41 0x29
#define CMD55  0x37
#define CMD58  0x3A

void sdc_init();
char sdc_cmd(unsigned char cmd, unsigned long arg);
