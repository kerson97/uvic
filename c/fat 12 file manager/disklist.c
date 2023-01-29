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
int checkNthBit(int inputNum, int numBit);
void removeSpaces(char *string);
void print_date_time(int directory_entry_startPos);
void traverseDirectory(FILE *fp, int address, char *dir);
unsigned long clusterOffset(short cluster);

int fileCount=0;


int main(int argc, char *argv[]){

//open image file
FILE *filePointer = fopen(argv[1], "rb");

printf("\nRoot\n");
printf("===========================================\n\n");
for(int i=0; i<14; i++){
    traverseDirectory(filePointer, 0x2600+512*i, "/");
}
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

void removeSpaces(char *string){
    char* temp = string;
    do{
        while(*temp==' '){
            ++temp;
        }
    } while(*string++ = *temp++);
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
                    printf("\n%s\n", new_dir);
                    printf("===========================================\n\n");
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
                printf("D %10d %.20s    ",size, fileName);
                print_date_time(offset);
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
                printf("F %10d %.17s.%.3s    ", size, fileName, extension);
                print_date_time(offset);
                fileCount++;
                }
}
           
}

unsigned long clusterOffset(short cluster){
    long answer = (cluster+31)*512;
    return answer;
}

int checkNthBit(int inputNum, int numBit){
    int bitStatus;
    bitStatus = (inputNum>>numBit) &1;
    return bitStatus;
}

void print_date_time(int directory_entry_startPos){
	
	int time, date;
	int hours, minutes, day, month, year;
	
	time = (directory_entry_startPos + 22);
	date = (directory_entry_startPos + 24);
	
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


    
    

