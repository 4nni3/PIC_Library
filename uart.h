#include <xc.h>

#define BUFF_SIZE 60
#define NUL '\0'

#define TXSTA TX1STA
#define RCSTA RC1STA
#define RCSTAbits RC1STAbits
#define RCREG RC1REG
#define TXREG TX1REG
#define SPBRG SP1BRG

void uart_init();
char uart_read();
void uart_write(char *data);
char uart_avaiable();

//Add interrupt
void uart_isr();
