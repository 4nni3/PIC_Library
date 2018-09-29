#include <xc.h>

#define SDC_CS LATAbits.LATA2

char sdc_init();
void sdc_open(char *filename);
void sdc_write(char *str, char b);
void sdc_close();
