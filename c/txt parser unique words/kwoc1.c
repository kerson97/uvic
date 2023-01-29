#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emalloc.h"
#include "listy.h"

#define MAX_WORDS 500
#define MAX_WORD_LEN 20
#define MAX_LINE_LEN 80

char words[MAX_WORDS][MAX_LINE_LEN];
char unique[MAX_WORDS][MAX_WORD_LEN];
char excludeWords[MAX_WORDS][MAX_WORD_LEN];

int num_words = 0;
int i;
int lines=0;
int numExclusionWords=0;

//declaring function prototypes

int num_occurrences(char *haystack, char *needle);
int longestWord(char array[][MAX_WORD_LEN]);
void make_upper(char array[MAX_WORDS][MAX_WORD_LEN], int size);
int numlines(FILE *p);
void store_the_words(char *lines);
int alphabetanize(const void *pa, const void *pb);

int main(int argc, char *argv[]) {

//simple command line handling

int input, exc;
if(strcmp(argv[1],"-e")==0){
    exc=2;
    input=3;
}
else {
    exc=3;
    input=1;
}

//open files depending on command line arguments

   FILE *input_file;
   input_file=fopen(argv[input], "r");

if(argc>=3){
   FILE *exclusion;
   exclusion=fopen(argv[exc], "r");

//count lines in excluded file
numExclusionWords= numlines(exclusion);

//store exclusion words in array
exclusion=fopen(argv[exc], "r");

for(i=0; i<numExclusionWords; i++){
    fgets(excludeWords[i],MAX_LINE_LEN,exclusion);
    excludeWords[i][strlen(excludeWords[i])-1] = '\0';
    }
}

//store exclusion words in a linked-list
    node_t *temp_node = NULL;
    node_t *head = NULL;

        for (i = 0; i < numExclusionWords; i++) {
        temp_node = new_node(excludeWords[i]);
        head = add_end(head, temp_node);
    }


//count lines in input array
lines=numlines(input_file);

//store lines in array
input_file=fopen(argv[input], "r");
for(i=0; i<lines; i++){
fgets(words[i], MAX_LINE_LEN, input_file);
char *pos;
if ((pos=strchr(words[i], '\n')) != NULL)
    *pos = '\0';
}

//store words in array

for(i=0; i<lines; i++){
store_the_words(words[i]);
}

//find and remove exclusion words
int j=0;
int k=0;
for(i=0; i<numExclusionWords; i++){
    for(j=0; j<num_words; j++){
     if(strcmp(unique[j+1],excludeWords[i])==0 && strcmp(unique[j],excludeWords[i])==0){
                num_words=num_words-2;
                for(int k=j; k<num_words; k++){
                strncpy(unique[k],unique[k+2], MAX_WORD_LEN);
                }
     }
        else if(strcmp(unique[j],excludeWords[i])==0){
            num_words--;
            for(int k=j; k<num_words; k++){
                strncpy(unique[k],unique[k+1], MAX_WORD_LEN);
                }
        }
    }
}

//sort alphabetically
qsort(unique, num_words, MAX_WORD_LEN, alphabetanize);

//make words array all CAPS
make_upper(unique, num_words);

//final print

int w,l,q=0;
for(w=0; w<num_words; w++){
    for(l=0; l<lines; l++){
    if(strncasecmp(unique[w], unique[w-1],MAX_WORD_LEN)!=0 &&(num_occurrences(words[l],unique[w])!=0)){

    //determine spacing between 'strings'
    int spaces=((longestWord(unique))+1)-strlen(unique[w]);

        if(num_occurrences(words[l],unique[w])>1){
            printf("%s%*s %s (%d*)\n",unique[w],spaces," ",words[l],l+1);}
        else
            printf("%s%*s %s (%d)\n",unique[w],spaces," ",words[l],l+1);
        }
    }
}

}//end of main

int numlines(FILE *p){
char c;
int total=0;
while((c=fgetc(p))!=EOF){
if(c=='\n')total++;
}
return total;
}
void store_the_words(char *line){
    char buffer[MAX_LINE_LEN];
    char *t;

    strncpy(buffer,line, MAX_LINE_LEN);
    t = strtok(buffer, " ");

    while (t != NULL) {

        strncpy(unique[num_words],t, MAX_WORD_LEN);

        num_words++;
        t = strtok(NULL, " ");
    }
    }
void make_upper(char array[][MAX_WORD_LEN], int size){
int i,j;

for(i=0; i<size && array[i][0]!='\0'; i++){
for(j=0; j<MAX_LINE_LEN && array[i][j]!='\0'; j++){
array[i][j]=toupper(array[i][j]);
}
}

}
int longestWord(char array[][MAX_WORD_LEN]){
int longest=0;
int i;

for(i=0; i<num_words; i++){
if(strlen(array[i])>longest){
longest=strlen(array[i]);
}
}
return longest;
}
int alphabetanize(const void *pa, const void *pb){
return strcmp(pa, pb);
}
int num_occurrences(char *haystack, char *needle){
    char buffer[MAX_LINE_LEN];
    char *t;
    int  num_occur = 0;

    strncpy(buffer, haystack, MAX_LINE_LEN);

    t = strtok(buffer, " \n");

    while (t != NULL) {
        if (strncasecmp(needle, t, MAX_LINE_LEN) == 0){
        num_occur++;
        }

        t = strtok(NULL, " \n");

}

    return (num_occur);
}
