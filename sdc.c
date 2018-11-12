#include <string.h>
#include "sdc.h"
#include "spi.h"

#define ERROR -1
#define OK 0

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
    unsigned char  FileName[11];//0-10
    unsigned char  Attributes;//11
    unsigned char  Reserved;
    unsigned char  CreationTimeTenths;
    unsigned short CreationTime;//14-15
    unsigned short CreationDate;
    unsigned short LastAccessDate;
    unsigned short Reserved2;//20-21
    unsigned short LastWriteTime;//22-23
    unsigned short LastWriteDate;
    unsigned short FirstCluster;//26-27
    unsigned long  FileSize;//28-31
};

bit notSDHC;
unsigned char buff[512];

char cmd(unsigned char cmd, unsigned long arg){

    char crc = 0xFF;
    if(cmd==CMD0) crc = 0x95;
    if(cmd==CMD8) crc = 0x87;
    
    while(spi_transfer(0xFF)!=0xFF);

    spi_transfer(cmd|0x40);
    spi_transfer(arg>>24);
    spi_transfer(arg>>16);
    spi_transfer(arg>>8);
    spi_transfer(arg);
    spi_transfer(crc);

    char r;
    short x=0;
    do{
    	r = spi_transfer(0xFF);
        x++;
    }while(r&0b10000000 && x<30000);
    if(x>=30000) return 0xFF;
    return r;
}
void read(unsigned long sector){
    if(notSDHC) sector<<=9;
    
    if(cmd(CMD17, sector)) return;
    
    while(spi_transfer(0xFF)!=0xFE) __delay_us(100);
    for(short i=0; i<512; i++){
        buff[i] = spi_transfer(0xFF);
    }
    spi_transfer(0xFF); //CRC
    spi_transfer(0xFF); //CRC
}
void write(unsigned long sector){
    if(notSDHC) sector<<=9;

    if(cmd(CMD24, sector)) return;
    spi_transfer(0xFE);
    for(short i=0; i<512; i++){
        spi_transfer(buff[i]);
    }
    spi_transfer(0xFF);//CRC
    spi_transfer(0xFF);//CRC
    
    while(spi_transfer(0xFF)!=0xFF);
}
unsigned long  DirEntry_P;
unsigned long  Data_P;
unsigned long  Fat_P;

unsigned short DirEntrySector_SU;
unsigned short Cluster1Sector_SU;
unsigned long  Fat1Sector_SU;

unsigned short DirEntryIndex;
unsigned long  FileSeekP;
unsigned short FirstFatno;
void fat_para_read(){
    union {
        unsigned char c[4];
        unsigned long l;
    } dt ;
    
    read(0);
    
    for(char i=0; i<4; i++){
        dt.c[i] = buff[454+i];
    }
    read(dt.l);
    
    struct FAT_PARA *fat = (struct FAT_PARA *)buff;

    Cluster1Sector_SU = fat->SectorsPerCluster;

    Fat1Sector_SU = fat->SectorsPerFat;

    Fat_P = fat->SecterPerBPB +dt.l;

    DirEntry_P = Fat_P +Fat1Sector_SU *fat->FatCount;

    DirEntrySector_SU = fat->RootDirEntryCount *32 /512;
    Data_P = DirEntry_P +DirEntrySector_SU;
}
char sdc_init(){
    char r;

    spi_init();

    SDC_CS = 1;
    for(char i=0; i<10; i++) spi_transfer(0xFF);
    SDC_CS = 0;
    
    if(cmd(CMD0, 0)!=0x01) return ERROR;
    
    unsigned long a = 0;
    if(cmd(CMD8, 0x1AA)!=0x01) return ERROR;
    spi_transfer(0xFF);
    spi_transfer(0xFF);
    spi_transfer(0xFF);
    if(spi_transfer(0xFF)==0xAA){//version2
        a = 0x40000000;
    }
    do{
        __delay_ms(1);
        cmd(CMD55, 0);
        r = cmd(ACMD41, a);
    }while(r==0x01);
    if(r) return ERROR;
    
    if(cmd(CMD58, 0)){//error
        return ERROR;
    }
    if(spi_transfer(0xFF)&0b01000000){//SDHC
        notSDHC = 0;
    }else{
        notSDHC = 1;
    }
    spi_transfer(0xFF);
    spi_transfer(0xFF);
    spi_transfer(0xFF);

    
    if(cmd(CMD16, 512)) return ERROR;
    
    fat_para_read();
    
    return OK;
}
char search_file(char *name){
    short c=-1;
    DirEntryIndex = 0;
    for(short i=0; i<DirEntrySector_SU; i++){
        read(DirEntry_P+i);

        for(char j=0; j<16; j++){//j個目のエントリ
            c++;

            struct DIR_ENTRY *inf = (struct DIR_ENTRY *)&buff[32*j];
            
            if(inf->FileName[0]==0xE5){
                if(DirEntryIndex ==0) DirEntryIndex = c;
                continue;
            }
            if(inf->FileName[0]==0x00){
                if(DirEntryIndex ==0) DirEntryIndex = c;

                return ERROR;
            }
            if(memcmp(inf->FileName, name, 11)==0){
                DirEntryIndex = c;
                FirstFatno = inf->FirstCluster;
                FileSeekP = inf->FileSize;

                return OK;
            }
        }
    }
    return ERROR;
}


//クラスタ番号を返す
unsigned short search_fat(){
    for(short i=0; i<Fat1Sector_SU; i++){//1個のFAT中のi個目のセクタ
        read(Fat_P +i);

        for(short j=0; j<512; j+=2){//セクタ中のj番地
            if( (buff[j]|buff[j+1]) ==0 ){//空を発見
                buff[j]=0xFF;//最後って表す
                buff[j+1]=0xFF;

                write(Fat_P +i);
                write(Fat_P +i +Fat1Sector_SU);

                return (i *512 +j) /2;
            }
        }
    }
    return OK;
}

//no: 最初のクラスタの位置
void entry_make(unsigned short no, char *name){
    unsigned long p = DirEntry_P + DirEntryIndex *32 /512;

    read(p);
    
    struct DIR_ENTRY *inf = (struct DIR_ENTRY *)&buff[(DirEntryIndex%16) *32];

    if(no!=0){
        memset(inf, 0x00, 32);
        memcpy(inf->FileName, name, 11);
        inf->Attributes = 0x20;
        inf->FirstCluster = no;
        
        FileSeekP = 0;
        FirstFatno = no;
    }else{
        //update
        inf->FileSize = FileSeekP;
    }

    write(p);

}
void sdc_open(char *filename){
    SDC_CS = 0;
    spi_transfer(0xFF);
    if(search_file(filename)){
        entry_make(search_fat(), filename);
    }
}
void sdc_close(){
    entry_make(0, 0);
    SDC_CS = 1;
}
unsigned short ClusterN;
void next_fat_read(){
    union {
        unsigned char c[2];
        unsigned short l;
    } no;
    
    unsigned long secter_p = Fat_P + (ClusterN *2 /512);
    short x_inSecter = (ClusterN %256) *2;
    read(secter_p);
    
    no.c[0] = buff[x_inSecter];
    no.c[1] = buff[x_inSecter+1];
    ClusterN = no.l;
    
    if(ClusterN>=0xFFF7){//次がない場合
        ClusterN = search_fat();//新しいの見つける

        //記録する
        no.l = ClusterN;
        buff[x_inSecter] = no.c[0];
        buff[x_inSecter+1] = no.c[1];
        write(secter_p);
        write(secter_p +Fat1Sector_SU);
    }
}

void sdc_write(char *str){
    
    ClusterN = FirstFatno;
    unsigned short q = FileSeekP /512 /Cluster1Sector_SU;//データの最後はq個目のクラスタ
    
    for(unsigned short i=0; i<q; i++) next_fat_read();

    //クラスタの最初のセクタの位置
    unsigned long dtCluster_P = Data_P + (ClusterN -2) *Cluster1Sector_SU;

    //シーク位置はクラスタ中p個目のセクタ
    unsigned short p = (FileSeekP /512) %Cluster1Sector_SU;

    //シーク位置はセクタ中x個目
    unsigned short x = FileSeekP %512;

    read(dtCluster_P +p);
    
    for(char i=0; str[i]!='\0'; i++){
        buff[x] = str[i];
        x++;
        FileSeekP++;
        if(x>=512){//次のセクタに入った
            write(dtCluster_P +p);
            p++;//次のセクタ
            if(p>=Cluster1Sector_SU){//次のクラスタに入った
                next_fat_read();
                dtCluster_P = Data_P +(ClusterN -2) *Cluster1Sector_SU;
                p = (FileSeekP /512) %Cluster1Sector_SU;
            }
            read(dtCluster_P +p);
            x = 0;
        }
    }
    write(dtCluster_P +p);
}
