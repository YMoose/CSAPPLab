#include "cachelab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#define MAXL 8192
char buf[MAXL];

struct line{
	char valid;
	long tag;
	int prev;
	int next;
};
struct set{
	struct line* l;
	int head;
	int tail;
};

int handlearg(int argc, char** argv, unsigned int *pset, unsigned int *pline, unsigned int *pbit, FILE** tracefile);
void printusage(void);
int strtouint(char* string, size_t len , unsigned int* num);
void analyseline(char* string, char* pop, long* paddr, unsigned int* psize);
int cachesearch(struct set* pcacheset, unsigned int Eline, long tag);
int cacheload(struct set* pcacheset, long tag, int LRUline);
int mygetline(FILE* file, char* buf);
void changepriority(struct set* pcacheset, int index);

int main(int argc, char** argv)
{
	int i;
	unsigned int sbit = 0;
	unsigned int Eline = 0;
	unsigned int bbit = 0;
	int hits = 0;
	int misses = 0;
	int evictions = 0;
	FILE* tracefile;
	if(handlearg(argc, argv, &sbit, &Eline, &bbit, &tracefile)){
		printusage();
		exit(0);
	}

	long bmask = ~(((~0)>>bbit)<<bbit);
	long smask = ~((((~0)>>(sbit+bbit))<<(sbit+bbit))|bmask);
	long tagmask = ~(smask|bmask);

//	build cache
	struct set* cachesets = (struct set*)malloc((1<<sbit)*sizeof(struct set));
	for(i = 0; i < (1<<sbit); i++){
		cachesets[i].l = (struct line*)malloc(Eline*sizeof(struct line));
		cachesets[i].head = 0;
		cachesets[i].tail = Eline-1;
		int j = 0;
		cachesets[i].l[0].prev = Eline-1;
		cachesets[i].l[0].next = Eline>1?1:0;
		cachesets[i].l[0].valid = 0;
		if(Eline>1){
			for(j = 1; j < Eline-1; j++){
				cachesets[i].l[j].prev = j-1;
				cachesets[i].l[j].next = j+1;
				cachesets[i].l[i].valid = 0;
			}
			cachesets[i].l[Eline-1].prev = Eline-2;
			cachesets[i].l[Eline-1].next = 0;
			cachesets[i].l[Eline-1].valid = 0;
		}
	}

	if(argc == 10){
		while(mygetline(tracefile,buf)){
 			char op = 0;
			long addr = 0;
			unsigned int size = 0;
			int LRUline = Eline;
			analyseline(buf,&op,&addr,&size);
			printf("%c %lx,%d",op,addr,size);
			LRUline = cachesearch(&cachesets[(addr&smask)>>bbit], Eline, addr&tagmask);
			if(LRUline == -1){
				hits++;
				printf(" hit");

			}
			else{
				misses++;
				printf(" miss");
				if(cacheload(&(cachesets[(addr&smask)>>bbit]),addr&tagmask,LRUline)){
					evictions++;
					printf(" eviction");
				}
			}
			if(op == 'M'){
				hits++;
				printf(" hit");
			}
			printf(" \n");
		}
	}
	else{
		while(mygetline(tracefile,buf)){
			char op = 0;
			long addr = 0;
			unsigned int size = 0;
			int LRUline = Eline;
			analyseline(buf,&op,&addr,&size);
			LRUline = cachesearch(&cachesets[addr&smask], Eline, addr&tagmask);
			if(LRUline == -1){
				hits++;
			}
			else{
				misses++;
				if(cacheload(&cachesets[addr&smask],addr&tagmask,LRUline)){
					evictions++;
				}
				if(op == 'M'){
					hits++;
				}
			}
		}
	}

    printSummary(hits, misses, evictions);
    return 0;
}

/*
 * handlearg: handle command pline args, if ok return 0, else return 1;
 */
int handlearg(int argc, char** argv, unsigned int *pset, unsigned int *pline, unsigned int *pbit, FILE** tracefile){
	if((argc == 9 ||(argc == 10 && strcmp(argv[1],"-v") == 0)) &&
		(strcmp(argv[argc-8], "-s") == 0) &&
		(strcmp(argv[argc-6], "-E") == 0) &&
		(strcmp(argv[argc-4], "-b") == 0) &&
		(strcmp(argv[argc-2], "-t") == 0))
	{
		if(strtouint(argv[argc-7], strlen(argv[argc-7]), pset) ||
		   strtouint(argv[argc-5], strlen(argv[argc-5]), pline) ||
		   strtouint(argv[argc-3], strlen(argv[argc-3]) ,pbit))
		{
			return 1;
		}
		if((*tracefile = fopen(argv[argc-1], "r"))==NULL){
			printf("%s : No such file or directory\n", argv[argc-1]);
			return 1;
		}
		return 0;
	}
	else{
		return 1;
	}
}

/*
 *	printusage
 */
void printusage(void){
	printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
	printf("Options:\n");
	printf("  -h         Print this help message.\n");
	printf("  -v         Optional verbose flag.\n");
	printf("  -s <num>   Number of set index bits.\n");
	printf("  -E <num>   Number of lines per set.\n");
	printf("  -b <num>   Number of block offset bits.\n");
	printf("  -t <file>  Trace file.\n");
	printf("\n");
	printf("Examples:\n");
	printf("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
	printf("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
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

/*
 *
 */
int mygetline(FILE* file, char* buf){
	char c = 0;
	int index = 0;
	while((c = fgetc(file))!=EOF && index < MAXL){
		if(c == '\n'){
			buf[index] = '\0';
			return 1;
		}
		if(index == 0 && c==' ') continue;
		buf[index] = c;
		index++;
	}
	return 0;
}

/*
 * analyseline: analyse line ,get the operation, address and size
 */
void analyseline(char* string, char* pop, long* paddr, unsigned int* psize){
	int index = 0;
	unsigned int getcnt = 0;
	while(string[index]!='\0' && string[index]!='\n' && index < MAXL && getcnt < 3){
		if(string[index]==' ' || string[index] == ','){
			index++;
			continue;
		}
		if(getcnt<1){
			*pop = string[index];
			getcnt = 1;
			index++;
			continue;
		}
		if(getcnt<2){
			while(string[index]!=' '&&string[index] != ','){
				int num = 0;
				if(string[index]<='9'&&'0'<=string[index]){
					num = string[index] - '0';
				}
				else if(string[index]<='z'&&'a'<=string[index]){
					num = string[index] - 'a' + 10;
				}
				else if(string[index]<='Z'&&'A'<=string[index]){
					num = string[index] - 'A' + 10;
				}
				else{
					printf("Bug in file: %s",buf);
					exit(0);
				}
				*paddr = (*paddr<<4) + num;
				index++;
			}
			getcnt = 2;
			continue;
		}
		if(getcnt<3){
			sscanf(buf+index,"%d",psize);
			getcnt = 3;
		}
	}
	return ;
}

/*
 * cachesearch: search @tag return -1 if hits else return LRUindex;
 */
int cachesearch(struct set* pcacheset, unsigned int Eline, long tag){
	int index = pcacheset->head;
	do{
		if(pcacheset->l[index].valid > 0 && pcacheset->l[index].tag == tag){
			if(pcacheset->head != index){
			changepriority(pcacheset,index);
			}
			return -1;
		}
		index = pcacheset->l[index].next;
	}while(index!=pcacheset->head);
	return pcacheset->tail;
}

/*
 * cashload: load mem to cache return 1 if eviction occur else return 0;
 */
int cacheload(struct set* pcacheset, long tag, int LRUline){
	int evic = pcacheset->l[LRUline].valid;
	pcacheset->l[LRUline].valid = 1;
	pcacheset->l[LRUline].tag = tag;
	changepriority(pcacheset, LRUline);
	return evic;
}

/*
 *
 */
void changepriority(struct set* pcacheset, int index){
	if(pcacheset->tail == index){
		pcacheset->tail = pcacheset->l[index].prev;
		pcacheset->head = index;
	}
	else{
		pcacheset->l[pcacheset->l[index].prev].next = pcacheset->l[index].next;
		pcacheset->l[pcacheset->l[index].next].prev = pcacheset->l[index].prev;
		pcacheset->l[index].next = pcacheset->head;
		pcacheset->l[pcacheset->head].prev = index;
		pcacheset->l[index].prev = pcacheset->tail;
		pcacheset->l[pcacheset->tail].next = index;
		pcacheset->head = index;
	}
}
