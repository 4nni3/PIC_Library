#include "uart.h"

char uart_buff[BUFF_SIZE];
char wptr=0;
char rptr=0;

void uart_init(){
    TXSTA = 0b00100100; //SYNC=0, TX9=0, BRGH=1
    RCSTA = 0b10010000; //TX9=0, CREN=1
    SPBRG = 207; //9600bps
    
    RCIF = 0;//INTR FLAG
    
    RCIE = 1;//INTR ENABLE
    PEIE = 1;
    GIE = 1;
    
}

char uart_read(){
    if(rptr==wptr) return NUL;
    char r = uart_buff[rptr];
    rptr = (rptr+1) %BUFF_SIZE;
    return r;
}

void uart_write(char *data){
    for(unsigned char i=0; data[i]!=NUL; i++){
        while(TXIF==0);
        TXREG = data[i];
    }
}
char uart_avaiable(){
    return (wptr-rptr +BUFF_SIZE) %BUFF_SIZE;
}

//Add interrupt
void uart_isr(){
    if(RCIF){
        if(RCSTAbits.OERR){
            RCSTAbits.SPEN = 0;
            NOP();
            RCSTAbits.SPEN = 1;
        }
        char wptr_ = (wptr+1) %BUFF_SIZE;
        if(wptr_!=rptr){
            uart_buff[wptr] = RCREG;
            wptr = wptr_;
        }
        RCIF = 0;
    }
}
