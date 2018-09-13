#include <xc.h>
#include "uart.h"

char buff[BUFF_SIZE];
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
    if(rptr==wptr) return NULL;
    char r = buff[rptr];
    rptr = (rptr+1) %BUFF_SIZE;
    return r;
}

void uart_write(char *data){
    for(unsigned char i=0; data[i]!=NUL; i++){
        while(TXIF==0);
        TX1REG = data[i];
    }
}
char uart_avaiable(){
    return (wptr-rptr +BUFF_SIZE) %BUFF_SIZE;
}

//Add interrupt
void uart_isr(){
    if(RCIF){
        char wptr_ = (wptr+1) %BUFF_SIZE;
        if(wptr_!=rptr){
            buff[wptr] = RCREG;
            wptr = wptr_;
        }
        if(RCSTAbits.OERR){//if get error
            RCSTAbits.CREN = 0;
            while(RC1STAbits.OERR);
            RCSTAbits.CREN = 1;
        }
        RCIF = 0;
    }
}
