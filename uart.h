#define BUFF_SIZE 80

#define TXSTA TX1STA
#define RCSTA RC1STA
#define RCSTAbits RC1STAbits
#define RCREG RC1REG
#define SPBRG SP1BRG

char buff[BUFF_SIZE];
char wptr;
char rptr;

void uart_init();
char uart_read();
void uart_write(char *data);
char uart_avaiable();

//Add interrupt
void uart_isr();
