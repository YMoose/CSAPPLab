#include "cachelab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

int handlearg(int argc, char** argv, unsigned int *set, unsigned int *line, unsigned int *bit, FILE* tracefile);
void printusage(void);
int strtouint(char* string, size_t len , unsigned int* num);

int main(int argc, char** argv)
{
	unsigned int sset = 0;
	unsigned int Eline = 0;
	unsigned int bbit = 0;
	FILE* tracefile;
	if(handlearg(argc, argv, &sset, &Eline, &bbit, tracefile)){
		printusage();
		exit(0);
	}

    printSummary(0, 0, 0);
    return 0;
}

/*
 * handlearg: handle command line args, if ok return 0, else return 1;
 */
int handlearg(int argc, char** argv, unsigned int *set, unsigned int *line, unsigned int *bit, FILE* tracefile){
	if((argc == 9 ||(argc == 10 && strcmp(argv[1],"-V") == 0)) &&
		(strcmp(argv[argc-8], "-s") == 0) &&
		(strcmp(argv[argc-6], "-E") == 0) &&
		(strcmp(argv[argc-4], "-b") == 0) &&
		(strcmp(argv[argc-2], "-t") == 0))
	{
		if(strtouint(argv[argc-7], strlen(argv[argc-7]), set) &&
		   strtouint(argv[argc-5], strlen(argv[argc-5]), line) &&
		   strtouint(argv[argc-3], strlen(argv[argc-3]) ,bit))
		{
			printusage();
			return 1;
		}
		return 0;
	}
	else{
		printusage();
		return 1;
	}
}

/*
 *
 */
void printusage(void){

}

/*
 * strtouint: @string to unsigned int @*num, if string is not a unsigned int return 1, ok return 0;
 */

int strtouint(char* string, size_t len , unsigned int* num){
	int i = 0;
	unsigned int temp = 0;
	for(i = 0; i < len; i++){
		char c = string[i];
		if(c <= '9' && '0' <= c){
			temp = temp * 10 + (c - '0');
		}
		else{
			return 1;
		}
	}
	*num = temp;
	return 0;
}
