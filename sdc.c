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
        //return;
    }
    while(spi_transfer(0xFF)!=0xFE);
    for(short i=0; i<SECTER; i++){
        buff[i] = spi_transfer(0xFF);
    }
    spi_transfer(0xFF);//CRC
    spi_transfer(0xFF);//CRC
}
void write(unsigned long sector){
    if(notSDHC) sector<<=9;

    cmd(CMD24, sector);
    spi_transfer(0xFE);
    for(short i=0; i<SECTER; i++){
        spi_transfer(buff[i]);
    }
    spi_transfer(0xFF);//CRC
    spi_transfer(0xFF);//CRC
}

unsigned long  Dir_Entry_StartP;//479
unsigned short DirEntry_SectorSU;
unsigned long  Data_Area_StartP;
unsigned long  Fat_Area_StartP;
unsigned short Cluster1_SectorSU;
unsigned long  SectorsPerFatSU;

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

    Cluster1_SectorSU = fat->SectorsPerCluster;

    SectorsPerFatSU = fat->SectorsPerFat;

    Fat_Area_StartP = dt.l + fat->SecterPerBPB;

    Dir_Entry_StartP = Fat_Area_StartP + SectorsPerFatSU * fat->FatCount;

    DirEntry_SectorSU = fat->RootDirEntryCount * 32 / SECTER;
    Data_Area_StartP = Dir_Entry_StartP + DirEntry_SectorSU;
    
    
    lcd_debug((char *)"PARA OK");

}
void sdc_init(){

    char r;

    spi_init();

    SDC_CS = 1;
    for(char i=0; i<10; i++) spi_transfer(0xFF);
    SDC_CS = 0;

    __delay_ms(1000);

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

    __delay_ms(1000);

    fat_para_read();
    __delay_ms(100);
    
    SDC_CS = 1;

}
char search_file(char name[11]){
    short c=0;
    char ans=0;
    DirEntryIndex = 0;
    for(short i=0; i<DirEntry_SectorSU; i++){
        read(Dir_Entry_StartP+i);

        for (char j=0; j<16; j++){
            c++;

            struct DIR_ENTRY *inf = (struct DIR_ENTRY *)&buff[j*32];

            if(inf->FileName[0]==0xE5){
                if(DirEntryIndex == 0) DirEntryIndex = c;
                continue;
            }
            if(inf->FileName[0]==0x00){
                if(DirEntryIndex == 0) DirEntryIndex = c;

                i = DirEntry_SectorSU;
                ans = 1;
                break;
            }
            if(memcmp(inf->FileName, name, 11)==0){
                DirEntryIndex = c;
                FileSeekP = 0;
                FileSize = inf->FileSize;

                FirstFatno = inf->FirstCluster;

                i = DirEntry_SectorSU;
                ans = 0;
                break;
            }
        }
    }
    return ans;
}
unsigned long search_fat(){
    unsigned long ans;

    for(short i=0; i<SectorsPerFatSU; i++){
        read(Fat_Area_StartP+i);

        for(short j=0; j<SECTER; j+=2){
            short x;
            for(char k=0; k<2; k++) x |= buff[j+k];
            if(x==0){
                buff[j]=0xFF;
                buff[j+1]=0xFF;

                write(Fat_Area_StartP+i);
                write(Fat_Area_StartP+i +SectorsPerFatSU);

                ans = (i*SECTER +j)/2;
            }
            i = SectorsPerFatSU;
            break;
        }
    }
    return ans;
}
unsigned long hhh;
void entry_make(unsigned long no, char *name){
    unsigned short x = DirEntryIndex-1;//9
    unsigned short y = SECTER/32;//16
    unsigned long p = Dir_Entry_StartP+(x/y);
    
    lcd_debug((char *)"MMM");
    hhh=p;
    read(p);
    lcd_debug((char *)"M1");
    
    struct DIR_ENTRY *inf = (struct DIR_ENTRY *)&buff[(x%y)*32];

    if(no!=0){
        memset(inf, 0x00, 32);
        memcpy(inf->FileName,name,11);
        inf->Attributes = 0x20;
        inf->FirstCluster = (unsigned short)no;
        FileSeekP = 0;
        FileSize  = 0;
        FirstFatno = inf->FirstCluster;
    }else{
        inf->FileSize = inf->FileSize + AppendSize;
    }

    write(p);
    lcd_debug((char *)"M2");

}
void sdc_open(char name[11]){
    SDC_CS = 0;
    spi_transfer(0xFF);
    
    if(search_file(name)){
        unsigned long no = search_fat();
        entry_make(no,name);
    }
    FileSeekP = FileSize;
    
    SDC_CS = 1;
    
}
void sdc_close(){
    
    SDC_CS = 0;
    spi_transfer(0xFF);
    
    if(AppendSize==0) return;
    entry_make(0, 0);
    
    SDC_CS = 1;
}

unsigned long next_fat_read(unsigned long fatno) {

    union {
        unsigned char c[4];
        unsigned long l;
    } no;

    unsigned long ans;

    // ＦＡＴ領域のデータを読込む
    unsigned long p = Fat_Area_StartP + fatno / (SECTER / 2);
    read(p);
    unsigned long x = (fatno % (SECTER / 2)) * 2;
    no.l = 0;
    
    no.c[0] = buff[x];
    no.c[1] = buff[x+1];
    
    ans = no.l;
    
    if(ans==0xFFFF){
        ans = search_fat();
        if(ans != 0){
            read(p);
            no.l = ans;
            buff[x] = no.c[0];
            buff[x+1] = no.c[1];
            write(p);
            write(p + SectorsPerFatSU);
        }
    }
    return ans;
}
void fatno_seek_conv(unsigned long *fatno){
     unsigned short p = FileSeekP / SECTER / Cluster1_SectorSU;

     *fatno = FirstFatno;
     for(short i=0 ; i<p ; i++){
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

    dtSP = Data_Area_StartP + (fatno - 2) * Cluster1_SectorSU;
    unsigned short p = (FileSeekP / SECTER) % Cluster1_SectorSU;

    read(dtSP + p);
    lcd_debug((char *)"W1");
    unsigned short x = FileSeekP % SECTER;

    for(char i=0; str[i]!='\0'; i++) {
        buff[x] = str[i];
        x++;
        FileSeekP++;
        AppendSize++;
        if (x >= SECTER){
            write(dtSP + p);
            lcd_debug((char *)"W2");
            p++;
            if(p >= Cluster1_SectorSU) {
                fatno_seek_conv(&fatno);

                dtSP = Data_Area_StartP + (fatno - 2) * Cluster1_SectorSU;
                p = (FileSeekP / SECTER) % Cluster1_SectorSU;
            }

            read(dtSP + p);
            lcd_debug((char *)"W3");
            x = 0;
        }
    }
    write(dtSP + p);
    
    lcd_debug((char *)"WriteOK");
    
    SDC_CS = 1;
    
}
