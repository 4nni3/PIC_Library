#include <xc.h>

#define SDC_CS LATAbits.LATA2

void sdc_init();
void sdc_put(char filename[11], char *str);
