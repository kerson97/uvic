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
#include<dirent.h>

//forward declarations

void traverseDirectory(FILE *fp, int address, char *dir);
int getNthFat(FILE * fp, int fatNum);
unsigned char* getString(FILE *fp, int offset, int numBytes);
int getData(FILE *fp, int offset, int numBytes);
void removeSpaces(char *string);
int checkNthBit(int inputNum, int numBit);
void uppercase(char *input);
int getNthFat(FILE * fp, int fatNum);
unsigned long clusterOffset(short cluster);
int searchCurDir(char *fileName);
int searchForPath(char *path, FILE *fp);
int freeSpace(FILE *fp);
int * getFatValue(int* threeBytes);
int makeDirectory(int atAddress, FILE *fp, char *dirName, char type);
void parsePath(char *path);
int firstEmptyFat(FILE *fp);
int copyToFat(FILE *fp, FILE *dest, int size);
void parseInputFile(char *input);
void setNthFat(FILE *fp, int set, int too);
u_int16_t swapBytes(const u_int16_t value);

//global variables
unsigned int fileCount=0;
unsigned int numSubDirs =0;
unsigned int bperSector;
unsigned int numSectors;
unsigned int fileArray[10000];
unsigned int directoryArray[100];
unsigned char directoryList[100][100];
unsigned int firstLogicalCluster;

unsigned char name[100];
unsigned char ext[50];

unsigned char *fileName;

int main(int argc, char *argv[]){

int mode; 
if(argc>3)mode = 3; //path mode
else mode = 2;      //root mode

 //open image file
FILE *filePointer = fopen(argv[1], "r+");
FILE *inputFile = fopen(argv[mode], "rb");    //fix this fucking bullshit


if(mode ==3){       //if path was specified, use that
char *fileName = argv[3];
char *path = argv[2];
unsigned int size;

uppercase(fileName);        //convert input to uppercase
uppercase(path);

for(int i=0; i<14; i++){                                //traverse, build directory array etc
    traverseDirectory(filePointer, 0x2600+512*i, "/");
}

unsigned int freespace = freeSpace(filePointer); //get free disk space

if(searchCurDir(fileName)){ //if file was found
   
    size =0;
    fseek(inputFile, 0L, SEEK_END);     //get size
    size = ceil(ftell(inputFile));
    printf("File Size: %d Free Space: %d\n", size, freespace);
        if(freespace<=size){                                    //check that disk has space
            printf("Error: Not enough disk space! \n");
            exit(1);                                               //exit if not
        }
    }
else{
    printf("File not found\n");
    exit(1);
    }

 parseInputFile(argv[3]); //parse input name into name/ext

unsigned int destination = searchForPath(path, filePointer);
if(destination !=0){                                        //if path was found, copy file to that path
    destination = (destination+31)*512;                      //find first logical cluster
    int dirAddress = makeDirectory(destination, filePointer, argv[3], 'F');   //make directory for file there
    
    copyToFat(filePointer, inputFile, size);    //copy to fat
    fseek(filePointer, dirAddress+26, SEEK_SET);        //write first logical cluster
    fwrite(&firstLogicalCluster, 1, 2, filePointer);

    fseek(filePointer, dirAddress+28, SEEK_SET);        //write size
    fwrite(&size, 1, 4, filePointer);
    rewind(filePointer);

                                                        //write dates(incomplete)
}

}

else{               //else use root
char *fileName = argv[2];
unsigned int size;

uppercase(fileName);        //convert input to uppercase

for(int i=0; i<14; i++){                                //traverse, build directory array etc
    traverseDirectory(filePointer, 0x2600+512*i, "/");
}

unsigned int freespace = freeSpace(filePointer); //get free disk space

if(searchCurDir(fileName)){ //if file was found
    size =0;
    fseek(inputFile, 0L, SEEK_END);     //get size
    size = ceil(ftell(inputFile));
    printf("File Size: %d Free Space: %d\n", size, freespace);
        if(freespace<=size){                                    //check that disk has space
            printf("Error: Not enough disk space! \n");
            exit(1);                                               //exit if not
        }
    }
else{
    printf("File not found\n");
    exit(1);
    }

    parseInputFile(argv[2]); //parse input name into name/ext

                                    
    int dirAddress = makeDirectory(9728, filePointer, argv[2], 'F');   //make directory for file in root
    
    copyToFat(filePointer, inputFile, size);    //copy to fat
    fseek(filePointer, dirAddress+26, SEEK_SET);        //write first logical cluster
    fwrite(&firstLogicalCluster, 1, 2, filePointer);

    fseek(filePointer, dirAddress+28, SEEK_SET);        //write size
    fwrite(&size, 1, 4, filePointer);
    rewind(filePointer);

                                                        //write dates(incomplete)
}

}

void traverseDirectory(FILE *fp, int address, char *dir){
    unsigned int clusterArray[100];
    unsigned int subNum=0;
    unsigned char *fileName;
    unsigned char *extension;
    unsigned short cluster;

    for(unsigned long offset = address; offset <= address+512; offset += 32){

        if(getData(fp, offset, 1)==0){
             for(int j=0; j<subNum; j++){           //recurse through sub directories if there are any
                    char new_dir[50];
                    strcpy(new_dir, dir);
                    strcat(new_dir, getString(fp, directoryArray[j], 8));
                    removeSpaces(new_dir);
                    strcat(new_dir, "/");
                    strcpy(directoryList[numSubDirs], new_dir);
                    numSubDirs++;
                    traverseDirectory(fp, clusterOffset(clusterArray[j]), new_dir);
            }
            return;
        }

        cluster = getData(fp, offset+26, 2);
        int size = getData(fp, offset+28, 4);
        unsigned char *fileName;
        unsigned char *extension;

        fileName = getString(fp, offset, 8);
        extension = getString(fp, offset+8, 3);

            if((getData(fp, offset+11, 1) >> 4)&1){ //check for subdirectories to recurse through later
                if(getData(fp, offset, 1)!= '.'){
                    directoryArray[subNum]=offset;
                    clusterArray[subNum] = cluster;
                    subNum++;
                }
            }

            if(checkNthBit(getData(fp, offset+11,1), 4)==1 && fileName[0]!='.'){    //directory
                removeSpaces(fileName);
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

int getNthFat(FILE * fp, int fatNum){
unsigned int location;
unsigned int high;
unsigned int low;
unsigned int rest;

//if fat num is even
if(fatNum%2==0){
low = getData(fp, floor(513+((3*fatNum)/2)),1);
low &= 0x0F;
low = low <<8;

rest = getData(fp, floor(512+((3*fatNum)/2)), 1);

location = rest | low;
}
//if odd
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
    unsigned int total = (*buff);
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
    unsigned int bitStatus;
    bitStatus = (inputNum>>numBit) &1;
    return bitStatus;
}

void uppercase(char *input){

    while(*input){
        *input = toupper(*input);
        input++;
    }
}

unsigned long clusterOffset(short cluster){
    long answer = (cluster+31)*512;
    return answer;
}

int searchCurDir(char *fileName){
    DIR *d;
    struct dirent *dir;
    

    d = opendir(".");
    if(d){
        while((dir = readdir(d))!=NULL){
            char *makeUpper = malloc(100*sizeof(char));
            makeUpper = dir->d_name;
            uppercase(makeUpper);
            if(strcmp(fileName, makeUpper)==0){
                printf("Found file '%s'in current directory\n", makeUpper);
                 closedir(d);
                return 1;}
        }

    }
    printf("Failed to find file in current directory\n");
    return 0;
}

int searchForPath(char *path, FILE *fp){        //returns first logical cluster of matching path
for(int i=0; i<numSubDirs; i++){
    if(strcmp(path, directoryList[i])==0){
        printf("Found path on FAT image\n");
        return  getData(fp, directoryArray[i]+26, 2);
    }
}
printf("Failed to find path on FAT image\n");
return 0;
}

int freeSpace(FILE *fp){
    unsigned int start = 515;                //skip first two entries (reserved)
    unsigned int *bytes = malloc(sizeof(int));
   unsigned  int  freeSectors = 0;
   unsigned  int entryNumber = 1;
   unsigned  int physicalNum=0;

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

int * getFatValue(int* threeBytes){
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

int makeDirectory(int atAddress, FILE *fp, char *dirName, char type){
 int start = getData(fp, atAddress, 2); //get 1 byte

 while(start !=0){              //find empty location
     atAddress+=32;
     start = getData(fp, atAddress, 2); //get 1 byte
 }
 parsePath(dirName);

 fseek(fp, atAddress, SEEK_SET);    //write name of directory at 1st 8 bytes of empty location
 fwrite(name, 1, 8, fp);
  fseek(fp, atAddress+8, SEEK_SET);
fwrite(ext, 1, 3, fp);
  

if(type == 'D'){
 fseek(fp, atAddress+11, SEEK_SET);     //set attribute to being a directory if type is D
 putc(16, fp);
}
 rewind(fp);
return atAddress;
}
void parsePath(char *path){
    int i,j;
    int length = strlen(path);

    for(i=0; i<length; i++){
        if(path[i] == '/'){
            for(j=i; j<length; j++){
                path[j] = path[j+1];
            }
            length--;
            i--;
        }
    }
}

int copyToFat(FILE *fp, FILE *source, int size){
unsigned int start = 516;                //skip first two entries (reserved)
unsigned int fatNum = 2;

unsigned int bytesLeft =size;
unsigned int bytesCopied =0;
unsigned  int usedClusters[10000];
unsigned  int counter=0;
unsigned   char buffer[1024];
unsigned  int threeBytes[3];
unsigned int physicalNumber;

unsigned int cluster = getNthFat(fp, 2);

 while(bytesLeft>0){            //while entire file has not been copied
    while(cluster != 0x00){
        cluster = getNthFat(fp, fatNum);
        fatNum++;
    } 
    if(firstLogicalCluster==0) firstLogicalCluster = fatNum;     //set firstlogicalcluster variable
    usedClusters[counter] = fatNum;
    counter++;
    physicalNumber = 512*(31+fatNum);
    //write data

    if(bytesLeft<=512){     //if few bytes are left, copy them then break
    fseek(fp, physicalNumber, SEEK_SET);        //seek to phys num in fp
    fseek(source, bytesCopied, SEEK_SET);       //seek to right spot in source
    fread(buffer, 1, bytesLeft, source);
    fwrite(buffer, 1, bytesLeft, fp);
    bytesLeft=0;
    bytesCopied+=bytesLeft;
    break;
    }
    fseek(fp, physicalNumber, SEEK_SET);
    fseek(source, bytesCopied, SEEK_SET);
    fread(buffer, 1, 512, source);
    fwrite(buffer, 1, 512, fp);
    bytesLeft-=512;
    bytesCopied+=512;
    
    cluster = getNthFat(fp, fatNum++);
  
 }
 //set correct fat entries
 int i=0;
     while(1){
        if(i==counter-1){        //set last piece to 0xFFF
        setNthFat(fp, usedClusters[i], 0xFFF);
        break;
     }
     setNthFat(fp, usedClusters[i], usedClusters[i+1]);
     i++;
 
 }

    printf("File copied to FAT succesfully!\n");
    return 1;

}

void parseInputFile(char *input){
    int i=0;
    int len = strlen(input);

    while(input[i]!= '.'){
        name[i] = input[i];
        i++;
    }
    len -=i;
    i++;
    int j=0;
    while(j<len-1){
        ext[j]=input[i];
        j++;
        i++;
    }
}

void setNthFat(FILE *fp, int set, int too){
unsigned int location;
unsigned int highLoc;
unsigned int highBytes;
unsigned int lowLoc;
unsigned char lowBytes;
unsigned int restLoc;
unsigned char restBytes;
unsigned char other;


if(set%2 ==0){
    lowBytes = (too >> 8)& 0x0F;
    restBytes = (too & 0xFF);

    fseek(fp, 512+(3*set/2), SEEK_SET);
    fputc(restBytes, fp);


    other =(getData(fp, 513+(3*set/2), 1) & 0xFF) + (lowBytes & 0x0F);
    fseek(fp, 513+ (3*set/2), SEEK_SET);
    fputc(other, fp);

}

else{

    highBytes = (too >> 4) & 0xFF;
    restBytes = (too <<4) & 0xF0;

    other = getData(fp, 512+(3*set/2),1) + restBytes;
    fseek(fp, 512+(3*set/2), SEEK_SET);
    fputc(other, fp);

    fseek(fp, 513+(3*set/2), SEEK_SET);
    fputc(highBytes, fp);

}

}

u_int16_t swapBytes(const u_int16_t value){
    u_int16_t result;
    result = value>> 8;
    result+= (value & 0xFF) <<8;
    return result;
}