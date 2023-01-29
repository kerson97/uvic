#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<math.h>

#define timeOffset 14 //offset of creation time in directory entry
#define dateOffset 16 //offset of creation date in directory entry

unsigned char* getString(FILE *fp, int offset, int numBytes);
int getData(FILE *fp, int offset, int numBytes);
int diskSize(FILE *fp);
int freeSpace(FILE *fp);
char *getVolume(FILE *fp);
int checkNthBit(int inputNum, int numBit);
void removeSpaces(char *string);
void print_date_time(int directory_entry_startPos);
int *getFatValue(int* threeBytes);
void traverseDirectory(FILE *fp, int address, char *dir);
unsigned long clusterOffset(short cluster);

int numSectors;
int bperSector;
int fileCount=0;

int main(int argc, char *argv[]){

char *volumeName;

//open image file
FILE *filePointer = fopen(argv[1], "rb");
printf("OS Name: %s\n", getString(filePointer, 3, 8));

if(strlen(getVolume(filePointer))!=0) volumeName = getVolume(filePointer); //determine volume name
else volumeName = getString(filePointer, 43,11);

printf("Volume Name: %s\n",volumeName);
printf("Total Size of Disk: %d\n", diskSize(filePointer));
printf("Free Space on Disk: %d\n", freeSpace(filePointer));

for(int i=0; i<14; i++){                                    //traverse and find files 
    traverseDirectory(filePointer, 0x2600+512*i, "/");
}
printf("Number of files in the image: %d\n", fileCount);

printf("Bytes per sector: %d\n", bperSector);
printf("Number of FAT Copies: %d\n", getData(filePointer, 16, 1));
printf("Sectors per FAT: %d\n\n", getData(filePointer, 22, 2));

}
unsigned char* getString(FILE *fp, int offset, int numBytes){
    unsigned char* buff = malloc(sizeof(unsigned char)*numBytes); 
    fseek(fp, offset, SEEK_SET);
    fread(buff, 1, numBytes, fp);
    rewind(fp);
    return buff;
}

int getData(FILE *fp, int offset, int numBytes){
    int *buff = calloc(sizeof(int), 1); 
    fseek(fp, offset, SEEK_SET);
    fread(buff,1, numBytes, fp);
    int total = (*buff);
    free(buff);
    rewind(fp);
    return total;
}

int diskSize(FILE *fp){
    int *sectorBytes = malloc(sizeof(int));
    int *totalSectors = malloc(sizeof(int));

    fseek(fp, 11, SEEK_SET);       //num bytes/sector
    fread(sectorBytes,1, 2, fp);
    bperSector = *sectorBytes;
    fseek(fp, 19, SEEK_SET);
    fread(totalSectors, 1, 2, fp);
    numSectors = *totalSectors;
    int total = (*sectorBytes)*(*totalSectors);   //*total sector count
    free(sectorBytes);
    free(totalSectors);
    rewind(fp);
    return total;
}

int freeSpace(FILE *fp){
    int start = 515;                //skip first two entries (reserved)
    int *bytes = malloc(sizeof(int));
    int  freeSectors = 0;           
    int entryNumber = 1;
    int physicalNum=0;
    
    while(physicalNum<=2879){          
       bytes[0]= getData(fp, start, 1);         //get 3 bytes
       bytes[1]= getData(fp, start+1, 1);
       bytes[2]= getData(fp, start+2, 1);
       bytes = getFatValue(bytes);              //convert to two entries

       entryNumber++; 
       physicalNum=31+entryNumber;              //calculate physical num
       if(bytes[0]==0 && (physicalNum<=2879)){
       freeSectors++;
       }
       entryNumber++;
       physicalNum = 31+entryNumber;
       if(bytes[1]==0 && (physicalNum<=2879)){
       freeSectors++;
       }

        start+=3;       //prepare to get next 3 bytes

  }
  rewind(fp);
    
    return freeSectors*512;

}

char * getVolume(FILE *fp){
    char *answer;
    int entrySize = 32;
    int numDirEntries = 16;
    int start = 9728;

    int i = 0;

    while(i<numDirEntries){
        if(strlen(getString(fp, start, 8))!=0){
            if(getData(fp, start+11, 1)==0x08){
                answer = getString(fp, start, 8);
                break;
            } 
        }
        i++;
        start+=entrySize;
    }
   
     return answer;
}

int checkNthBit(int inputNum, int numBit){
    int bitStatus;
    bitStatus = (inputNum>>numBit) &1;
    return bitStatus;
}

void removeSpaces(char *string){
    char* temp = string;
    do{
        while(*temp==' '){
            ++temp;
        }
    } while(*string++ = *temp++);
}

void print_date_time(int directory_entry_startPos){
	
	int time, date;
	int hours, minutes, day, month, year;
	
	time = (directory_entry_startPos + timeOffset);
	date = (directory_entry_startPos + dateOffset);
	
	//the year is stored as a value since 1980
	//the year is stored in the high seven bits
	year = ((date & 0xFE00) >> 9) + 1980;
	//the month is stored in the middle four bits
	month = (date & 0x1E0) >> 5;
	//the day is stored in the low five bits
	day = (date & 0x1F);
	
	printf("%d-%02d-%02d ", year, month, day);
	//the hours are stored in the high five bits
	hours = (time & 0xF800) >> 11;
	//the minutes are stored in the middle 6 bits
	minutes = (time & 0x7E0) >> 5;
	
	printf("%02d:%02d\n", hours, minutes);
	
	return ;	
}

void traverseDirectory(FILE *fp, int address, char *dir){
    int directoryArray[100];
    int clusterArray[100];
    int subNum=0;
    char *fileName;
    char *extension;
    unsigned short cluster;

    for(unsigned long offset = address; offset <= address+512; offset += 32){
        if(getData(fp, offset, 1)==0){
             for(int j=0; j<subNum; j++){
                    char new_dir[50];
                    strcpy(new_dir, dir);
                    strcat(new_dir, getString(fp, directoryArray[j], 8));
                    removeSpaces(new_dir);
                    strcat(new_dir, "/");
                    //printf("\n%s\n", new_dir);
                    //printf("===========================================\n\n");
                    subNum--;
                    traverseDirectory(fp, clusterOffset(clusterArray[j]), new_dir);
            }
            return;
        }

        cluster = getData(fp, offset+26, 2);
        int size = getData(fp, offset+28, 4);
        char *fileName;
        char *extension;

        fileName = getString(fp, offset, 8);
        extension = getString(fp, offset+8, 3);

            if((getData(fp, offset+11, 1) >> 4)&1){
                if(getData(fp, offset, 1)!= '.'){
                    directoryArray[subNum]=offset;
                    clusterArray[subNum] = cluster;
                    subNum++;
                }
            }

            if(checkNthBit(getData(fp, offset+11,1), 4)==1 && fileName[0]!='.'){    //directory
                removeSpaces(fileName);
                //printf("D %10d %.17s.%.3s ",size, fileName, extension);
                //print_date_time(offset);
            } 

            else if(
                getData(fp, offset+11, 1)!=0x0F &&   //attribute isnt 0x0F (long dir name)
                (getData(fp, offset+26, 2)!=0 || getData(fp, address+26, 2)!=1) && //first logical cluster isnt 0/1
                checkNthBit(getData(fp, offset+11, 1), 3)!=1 && //not volume label
                getData(fp, offset, 1) != 0xE5 &&    //not free
                getData(fp, offset, 1) != 0x00 &&
                fileName[0]!='.' //not a . 
                ){
                removeSpaces(fileName);
                //printf("F %10d %.17s.%.3s ", size, fileName, extension);
                //print_date_time(offset);
                fileCount++;
                }
}
           
}

unsigned long clusterOffset(short cluster){
    long answer = (cluster+31)*512;
    return answer;
}

int * getFatValue(int* threeBytes)

{
   int uv, wx, yz;
   
   uv = threeBytes[0];
   wx = threeBytes[1];
   yz = threeBytes[2];

   int x = wx & 0xf;
   int w = (wx >>4) & 0xf;
   threeBytes[0] = uv | (x << 8);
   threeBytes[1] = (yz << 4) | w;

   
   return threeBytes;

}
    

    
    

