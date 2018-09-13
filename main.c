

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _XTAL_FREQ 32000000

#define NUL '\0'

void checkMes(char *mes);

void sexToDec(char *sex, char *r);
void getMesItem(char *mes, char index, char *r);
void checkStatus(char *mes);


bit status;

void main(void) {
    
    //input
    TRISA = 0b00000000;
    TRISC = 0b00100011;
    
    //analog
    ANSELA = 0b00000000;
    ANSELC = 0b00000000;
    
    //IO
    LATA = 0;
    LATC = 0;
    
    //PSS
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 0;
    
    RXPPS = 0b00010101; //RC5->UART:RX
    RC4PPS = 0b00010100; //UART:TX->RC4
    
    //RC0 = Clock
    //RC1 = Data
    SSP1CLKPPS = 0b00010000; //RC0->SSP1:CLK
    RC0PPS = 0b00011000; //SSP1:CLK->RC0
    
    SSP1DATPPS = 0b00010001; //RC1->SSP1:Data
    RC1PPS = 0b00011001; //SSP1:Data->RC1
    
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCKbits.PPSLOCKED = 1;
    
    uart_init();
    
    
    uart_write((char *)"$PSRF103,0,0,1,1*25");
    uart_write((char *)"$PSRF103,1,0,0,1*25");
    uart_write((char *)"$PSRF103,2,0,0,1*26");
    uart_write((char *)"$PSRF103,3,0,0,1*27");
    uart_write((char *)"$PSRF103,4,0,1,1*21");
    uart_write((char *)"$PSRF103,5,0,0,1*21");
    uart_write((char *)"$PSRF103,6,0,0,1*22");
    uart_write((char *)"$PSRF103,7,0,0,1*23");
    uart_write((char *)"$PSRF103,8,0,0,1*2C");
    
    
    i2c_init();
    oled_init();
    oled_clear();
    oled_str((char *)"00:00:00");
    
    char flag = -1;
    char sentence[70];
    while(1){
        if(uart_avaiable()==0) continue;
        
        char data = uart_read();

        if(data=='$'){//start
            memset(sentence, NUL, sizeof(sentence));
            flag = 0;
        }
        
        if(flag != -1){//save
            sentence[flag] = data;
            flag++;
        }

        if(flag!=-1 && data=='\n'){//end
            checkMes(sentence);
            flag = -1;
            
        }
    }
    return;
}

void sexToDec(char *sex, char *r){
    double a = atof(sex);
    double b = (char)a+(a-(char)a)/3*5;
    sprintf(r, "%.5f", b);
}

void formatTime(char *str, char *r){
    char h[3];
    strncpy(h, str, 2);
    
    char m[3];
    strncpy(m, str+2, 2);
    
    char s[3];
    strncpy(s, str+4, 2);
    
    sprintf(r, "%s:%s:%s\0", h, m, s);
}
char timeStr[9];
void checkMes(char *mes){
    
    if(status==0){
        checkStatus(mes);
        if(status==0) return;
        else oled_clear();
    }
    
    if(strncmp(mes, "$GPRMC", 6)==0){
        char time[10];
        getMesItem(mes, 1, time);
        
        memset(timeStr, NUL, sizeof(timeStr));
        formatTime(time, timeStr);
        
        oled_str(timeStr);
    }else if(strncmp(mes, "$GPGGA", 6)==0){
      
    }
    
}


void interrupt isr(){
}

void getMesItem(char *mes, char index, char *r){
	char cnt=0;
	char p=0;
	for(char i=0; mes[i]!=NUL; i++){
		if(mes[i]==',') cnt++;
		else if(cnt==index){
			r[p]=mes[i];
			p++;
		}
		else if(cnt>index) break;
	}
    r[p] = NUL;
}

void checkStatus(char *mes){
    if(strncmp(mes, "$GPRMC", 6)!=0) return;
    
    char r[2];
    getMesItem(mes, 2, r);
    
    status = (r[0]=='A')?1:0;
}

// -----UART methods-----


// -----I2C methods-----


// -----OLED methods-----
