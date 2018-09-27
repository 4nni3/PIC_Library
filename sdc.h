#include <xc.h>

#define SDC_CS LATAbits.LATA2

void sdc_init();
void sdc_open(char name[11]);
void sdc_write(char *str);
void sdc_close();
