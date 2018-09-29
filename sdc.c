#include <stdio.h>
#include <string.h>
#include "sdc.h"
#include "spi.h"

#include "lcd.h"

#define SECTER 512

#define CMD0   0
#define CMD1   1
#define CMD8   8
#define CMD12  12
#define CMD13  13
#define CMD16  16
#define CMD17  17
#define CMD24  24
#define ACMD41 41
#define CMD55  55
#define CMD58  58

struct FAT_PARA{
    unsigned char  jump[3];//0-2
    unsigned char  oemId[8];//3-10
    unsigned short BytesPerSector;//11-12
    unsigned char  SectorsPerCluster;//13
    unsigned short SecterPerBPB;//14-15
    unsigned char  FatCount;
    unsigned short RootDirEntryCount;
    unsigned short TotalSectors;
    unsigned char  MediaType;
    unsigned short SectorsPerFat;
    unsigned short SectorsPerTrack;
    unsigned short HeadCount;
    unsigned long  HidddenSectors;
    unsigned long  TotalSectors32;
    unsigned long  SectorsPerFat32;
    unsigned short FlagsFat32;
    unsigned short VersionFat32;
    unsigned long  RootDirClusterFat32;
    unsigned char  Dumy[6];
    unsigned char  FileSystemType[8];
    unsigned char  BootCode[448];
    unsigned char  BootSectorSig0;
    unsigned char  BootSectorSig1;
};


struct DIR_ENTRY{
    unsigned char  FileName[11];
    unsigned char  Attributes;
    unsigned char  Reserved;
    unsigned char  CreationTimeTenths;
    unsigned short CreationTime;
    unsigned short CreationDate;
    unsigned short LastAccessDate;
    unsigned short FirstCluster;
    unsigned short LastWriteTime;
    unsigned short LastWriteDate;
    unsigned short Reserved2;
    unsigned long  FileSize;
};

bit notSDHC;

char buff[SECTER];

char cmd(unsigned char cmd, unsigned long arg){

    char crc = 0xFF;
    if(cmd==CMD0) crc = 0x95;
    if(cmd==CMD8) crc = 0x87;

    spi_transfer(cmd|0x40);
    spi_transfer(arg>>24);
    spi_transfer(arg>>16);
    spi_transfer(arg>>8);
    spi_transfer(arg);
    spi_transfer(crc);

    char r;
    //short x=0;
    do{
    	r = spi_transfer(0xFF);
        //x++;
    }while(r==0xFF /*&& x<30000*/);

    return r;
}
void read(unsigned long sector){
    if(notSDHC) sector<<=9;

    char r = cmd(CMD17, sector);
    if(r!=0x00){
        char c[10];
        sprintf(c, "C17Er%02X\0", r);
        lcd_debug(c);
        return;
    }
    while(spi_transfer(0xFF)!=0xFE);
    for(short i=0; i<SECTER; i++){
        buff[i] = spi_transfer(0xFF);
    }
    spi_transfer(0x01);//CRC
    spi_transfer(0x01);//CRC
}
void write(unsigned long sector){
    if(notSDHC) sector<<=9;

    cmd(CMD24, sector);
    spi_transfer(0xFE);
    for(short i=0; i<SECTER; i++){
        spi_transfer(buff[i]);
    }
    spi_transfer(0x01);//CRC
    spi_transfer(0x01);//CRC
}
unsigned long  DirEntry_P;
unsigned long  Data_P;
unsigned long  Fat_P;

unsigned short DirEntrySecter_SU;
unsigned short Cluster1Sector_SU;
unsigned long  Fat1Sector_SU;

unsigned short DirEntryIndex;
unsigned long FileSize;
unsigned long FileSeekP;
unsigned long AppendSize;
unsigned short FirstFatno;
void fat_para_read(){
    union {
        unsigned char c[4];
        unsigned long l;
    } dt ;

    read(0);

    for(char i=0 ; i<4 ; i++){
        dt.c[i] = buff[454+i];
    }

    read(dt.l);

    struct FAT_PARA *fat = (struct FAT_PARA *)buff;

    Cluster1Sector_SU = fat->SectorsPerCluster;

    Fat1Sector_SU = fat->SectorsPerFat;

    Fat_P = fat->SecterPerBPB +dt.l;

    DirEntry_P = Fat_P +Fat1Sector_SU *fat->FatCount;

    DirEntrySecter_SU = fat->RootDirEntryCount *32 /512;
    Data_P = DirEntry_P +DirEntrySecter_SU;
    
    lcd_debug((char *)"PARA OK");

}
void sdc_init(){

    char r;

    spi_init();

    SDC_CS = 1;
    for(char i=0; i<10; i++) spi_transfer(0xFF);
    SDC_CS = 0;

    lcd_debug((char *)"CMD0");
    r = cmd(CMD0, 0);
    if(r!=0x01){
        char b[6];
        sprintf(b, "Er%02X\0", r);
        lcd_debug(b);
        return;
    }
    lcd_debug((char *)"OK");


    lcd_debug((char *)"CMD8");
    if(cmd(CMD8, 0x1AA)==0x01){//version1
        lcd_debug((char *)"ver1");
    }else{
        lcd_debug((char *)"ver2");
    }
    spi_transfer(0xFF);
    spi_transfer(0xFF);
    spi_transfer(0xFF);
    spi_transfer(0xFF);

    lcd_debug((char *)"CMD55");
    do{
        __delay_ms(1);
        cmd(CMD55, 0);
        r = cmd(ACMD41, 0x40000000);
    }while(r==0x01);
    if(r!=0x00){
        char c[6];
        sprintf(c, "Er%02X\0", r);
        lcd_debug(c);
        return;
    }
    lcd_debug((char *)"OK");

    lcd_debug((char *)"CMD58");
    if(cmd(CMD58, 0)!=0x00){//error
        lcd_debug((char *)"Er");
        return;
    }
    if(spi_transfer(0xFF)&0b01000000){//SDHC
        notSDHC = 0;
        lcd_debug((char *)"SDHC");
    }else{
        notSDHC = 1;
        lcd_debug((char *)"SD");
    }
    spi_transfer(0xFF);
    spi_transfer(0xFF);
    spi_transfer(0xFF);

    lcd_debug((char *)"CMD16");
    if(cmd(CMD16, SECTER)!=0x00){//error
        char d[6];
        sprintf(d, "Er%02X\0", r);
        lcd_debug(d);
        return;
    }
    lcd_debug("OK");

    lcd_debug((char *)"Success!");

    fat_para_read();
}
char search_file(char name[11]){
    short c=0;
    char ans=0;
    DirEntryIndex = 0;
    for(short i=0; i<DirEntrySecter_SU; i++){
        read(DirEntry_P+i);

        for(char j=0; j<16; j++){
            c++;

            struct DIR_ENTRY *inf = (struct DIR_ENTRY *)&buff[32*j];

            if(inf->FileName[0]==0xE5){
                if(DirEntryIndex == 0) DirEntryIndex = c;
                continue;
            }
            if(inf->FileName[0]==0x00){
                if(DirEntryIndex == 0) DirEntryIndex = c;

                i = DirEntrySector_SU;
                ans = 1;
                break;
            }
            if(memcmp(inf->FileName, name, 11)==0){
                DirEntryIndex = c;
                FileSeekP = 0;
                FileSize = inf->FileSize;

                FirstFatno = inf->FirstCluster;

                i = DirEntrySector_SU;
                ans = 0;
                break;
            }
        }
    }
    return ans;
}
unsigned long search_fat(){
    unsigned long ans;
    char x;
    for(short i=0; i<Fat1Sector_SU; i++){
        read(Fat_P+i);

        for(short j=0; j<512; j+=2){
            x  = buff[j];
            x |= buff[j+1];

            if(x==0){
                buff[j]=0xFF;
                buff[j+1]=0xFF;

                write(Fat_P +i);
                write(Fat_P +i +Fat1Sector_SU);

                ans = (i*512+j)/2;

                i = Fat1Sector_SU;
                break;
            }
        }
    }
    return ans;
}
void entry_make(unsigned long no, char *name){
    unsigned long p = DirEntry_P + (DirEntryIndex-1)*32/512;

    read(p);
    
    struct DIR_ENTRY *inf = (struct DIR_ENTRY *)&buff[((DirEntryIndex-1)%16)*32];

    if(no!=0){
        memset(inf, 0x00, 32);
        memcpy(inf->FileName, name, 11);
        inf->Attributes = 0x20;
        inf->FirstCluster = (unsigned short)no;
        FileSeekP = 0;
        FileSize  = 0;
        FirstFatno = inf->FirstCluster;
    }else{
        inf->FileSize += AppendSize;
    }

    write(p);
    lcd_debug((char *)"M2");

}
void sdc_open(char name[11]){
    SDC_CS = 0;
    spi_transfer(0xFF);
    
    if(search_file(name)){
        unsigned long no = search_fat();
        entry_make(no, name);
    }
    FileSeekP = FileSize;
    
    SDC_CS = 1;
    
}
void sdc_close(){
    SDC_CS = 0;
    spi_transfer(0xFF);
    
    if(AppendSize!=0) entry_make(0, 0);

    DirEntrySecter_SU;
    Cluster1Sector_SU;
    Fat1Sector_SU;

    DirEntryIndex;
    FileSize;
    FileSeekP;
    AppendSize;
    FirstFatno;
    
    SDC_CS = 1;
}

unsigned short next_fat_read(unsigned long fatno) {

    union {
        unsigned char c[2];
        unsigned short l;
    } no;
    unsigned short ans;

    unsigned long p = Fat_P + fatno *2 /512;
    read(p);

    unsigned long x = (fatno %256) *2;
    no.l = 0;
    
    no.c[0] = buff[x];
    no.c[1] = buff[x+1];
    
    ans = no.l;
    
    if(ans==0xFFFF){//already yoyaku
        ans = search_fat();
        if(ans != 0){
            read(p);
            no.l = ans;
            buff[x] = no.c[0];
            buff[x+1] = no.c[1];
            write(p);
            write(p + Fat1Sector_SU);
        }
    }
    return ans;
}
void fatno_seek_conv(unsigned long *fatno){
     unsigned short p = FileSeekP /512 /Cluster1Sector_SU;

     *fatno = FirstFatno;
     for(unsigned short i=0 ; i<p; i++){
         *fatno = next_fat_read(*fatno);
     }
}

void sdc_write(char *str){
    
    SDC_CS = 0;
    spi_transfer(0xFF);
    
    lcd_debug((char *)"WriteSS");
    
    unsigned long dtSP;
    unsigned long fatno;

    fatno_seek_conv(&fatno);

    dtSP = Data_P + (fatno -2) *Cluster1Sector_SU;
    unsigned short p = (FileSeekP /512) %Cluster1Sector_SU;

    read(dtSP +p);
    lcd_debug((char *)"W1");
    unsigned short x = FileSeekP %512;

    for(char i=0; str[i]!='\0'; i++) {
        buff[x] = str[i];
        x++;
        FileSeekP++;
        AppendSize++;
        if (x>=512){
            write(dtSP +p);
            p++;
            if(p>=Cluster1Sector_SU) {
                fatno_seek_conv(&fatno);
                dtSP = Data_P +(fatno -2) *Cluster1Sector_SU;
                p = (FileSeekP /512) %Cluster1Sector_SU;
            }
            read(dtSP + p);
            x = 0;
        }
    }
    write(dtSP +p);
    
    lcd_debug((char *)"WriteOK");
    
    SDC_CS = 1;
}
