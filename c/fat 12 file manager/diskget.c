#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<math.h>
#include<ctype.h>

//forward declarations
int search(FILE *fp, char *name);
void traverseDirectory(FILE *fp, int address, char *dir);
int getNthFat(FILE * fp, int fatNum);
unsigned char* getString(FILE *fp, int offset, int numBytes);
int getData(FILE *fp, int offset, int numBytes);
void removeSpaces(char *string);
int checkNthBit(int inputNum, int numBit);
void uppercase(char *input);
void copyFile(FILE *fp, int address);
int getNthFat(FILE * fp, int fatNum);
unsigned long clusterOffset(short cluster);

//global variables
int fileCount=0;
int fileArray[10000];
char *fileName;

int main(int argc, char *argv[]){

 //open image file
FILE *filePointer = fopen(argv[1], "rb");

char *fileName = argv[2];
uppercase(fileName);        //convert input to uppercase

for(int i=0; i<14; i++){
    traverseDirectory(filePointer, 0x2600+512*i, "/");
}
search(filePointer, fileName);  
}

int search(FILE *fp, char *name){
    
    for(int i=0; i<fileCount; i++){                         //search file addresses for one that matches input string
        fileName = getString(fp, fileArray[i], 8);
        removeSpaces(fileName);
        strcat(fileName, ".");
        strncat(fileName, getString(fp, fileArray[i]+8, 3), 3);
        removeSpaces(fileName);
        if(strcmp(name, fileName)==0){                      //if yes, copy
            printf("File '%s' found, copying...\n\n", name);
            copyFile(fp, fileArray[i]);
            return 1;
        }
    }
    printf("File not found\n");     //else display error
    return 0;
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
            else if(
                getData(fp, offset+11, 1)!=0x0F &&   //attribute isnt 0x0F (long dir name)
                (getData(fp, offset+26, 2)!=0 || getData(fp, address+26, 2)!=1) && //first logical cluster isnt 0/1
                checkNthBit(getData(fp, offset+11, 1), 3)!=1 && //not volume label
                getData(fp, offset, 1) != 0xE5 &&    //not free
                getData(fp, offset, 1) != 0x00 &&
                fileName[0]!='.' //not a . 
                ){
                fileArray[fileCount] = offset;
                fileCount++;
                }
}
           
}

int getNthFat(FILE * fp, int fatNum){
int location;
int high;
int low;
int rest;

if(fatNum%2==0){
low = getData(fp, floor(513+((3*fatNum)/2)),1);
low &= 0x0F;
low = low <<8;

rest = getData(fp, floor(512+((3*fatNum)/2)), 1);

location = rest | low;
}
else{
high = getData(fp, floor(512+((3*fatNum)/2)), 1);
high &= 0xF0;
high = high >>4;

rest = getData(fp, floor(513+((3*fatNum/2))),1 );
rest = rest<<4;
location = rest | high;

}
return location;
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

int checkNthBit(int inputNum, int numBit){
    int bitStatus;
    bitStatus = (inputNum>>numBit) &1;
    return bitStatus;
}

void uppercase(char *input){
   
    while(*input){
        *input = toupper(*input);
        input++;
    }
}

void copyFile(FILE *fp, int address){

int cluster = getData(fp, address+26, 2); //first piece
int fileSize = getData(fp, address+28, 4);
float sectorsNeeded = fileSize/512;
int physicalNumber;
int readBytes =0;
int bytesLeft=fileSize;
char buffer[1024];

FILE *newFile;
newFile = fopen(fileName, "w");     //create empty file to copy contents

    while(cluster!=0xFFF){

            physicalNumber = (31+cluster)*512;             //ie 31+101 * 512
            if(fileSize-readBytes>=512){            //if less than 512 to go, write them
            fseek(fp, physicalNumber, SEEK_SET);
            fread(buffer, 1, 512, fp);
            fwrite(buffer, 1, 512, newFile);
            readBytes+=512;
            }
            else{                                       //if more, write 512b chunk
            fseek(fp, physicalNumber, SEEK_SET);
            fread(buffer, 1, fileSize-readBytes, fp);
            fwrite(buffer, 1, fileSize-readBytes, newFile);
            readBytes+=fileSize-readBytes;
            }

        //next cluster
        int nextPiece = getNthFat(fp, cluster);
        cluster= nextPiece;        //get next cluseter
     }
                
     printf("File copied succesfully.\n\n");
     printf("Name: '%s' Copied: %d bytes\n\n", fileName, readBytes);
     exit(1);
}

unsigned long clusterOffset(short cluster){
    long answer = (cluster+31)*512;
    return answer;
}
