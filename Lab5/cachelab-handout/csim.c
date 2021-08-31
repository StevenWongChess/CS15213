#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>

// address = t(tag) + s(set) + b(offset)
// line = v(valid) + tag + data(b bits)

#define MAX_CHAR 40
typedef struct _block{
	bool valid;
	unsigned long tag;
	int lru;
	// char* data;
}block;

int s;
int S; 
int E; 
int b;
int B; 
bool v;
int hit; 
int miss; 
int eviction;

void visit(int address, block *cache){
	unsigned long tag = address >> (s + b);
	// unsigned long set_index = (address >> b) % (1 << s);
	unsigned long set_index = (address >> b) & ((1 << s) - 1);
	// unsigned long offset = address & ((1 << b) - 1);

	for(int i = 0; i < E; i++){
		int j = set_index * E + i;
		if(cache[j].valid && cache[j].tag == tag){
			hit++;
			cache[j].lru = 0;
			return;
		}
	}
	miss++;
	for(int i = 0; i < E; i++){
		int j = set_index * E + i;
		if(!cache[j].valid){
			cache[j].valid = true;
			cache[j].tag = tag;
			cache[j].lru = 0;
			return;
		}
	}
	eviction++;
	int least_lru = -1;
	int index = 0;
	for(int i = 0; i < E; i++){
		int j = set_index * E + i;
		if(cache[j].lru > least_lru){
			least_lru = cache[j].lru;
			index = j;
		}
	}

	cache[index].tag = tag;
	// cache[index].valid = true;
	cache[index].lru = 0;
};

int main(int argc, char *argv[])
{
	// we need to read these arg
	v = false;  
	char* file_path;

	int ch;
	const char *help_info= "use flag v h s E b t\n";

	// parse command line arguments
	while((ch = getopt(argc, argv, ":vhs:E:b:t:")) != -1){
		switch(ch){
			case 'h':
				printf("%s", help_info);
				return -1;
			case 'v':
				v = true;
				break;
			case 's':
				s = atoi(optarg);
				S = 1 << s;
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				B = 1 << b;
				break;
			case 't':
				file_path = optarg;
				break;
			case ':':
				printf("missing arg for %c!\n", optopt);
				break;
			case '?':
				printf("unknown option %c!\n", optopt);
		}
	}
	// printf("%d %d %d %d\n%s\n", v, S, E, B, file_path);

	// initialize empty cache
	block *cache = malloc(S * E * sizeof(block));
	for(int i = 0; i < S * E; i++){
		cache[i].valid = false;
		cache[i].tag = 0;
		cache[i].lru = -1;
		// cache[i].data = malloc(B * sizeof(char));
	}

	// read the trace line by line
	FILE* file = fopen(file_path, "r");
	if(!file){
		printf("%s read fail!\n", file_path);
		return -1;
	}
	char line[MAX_CHAR];
	hit = miss = eviction = 0;

	while(fgets(line, MAX_CHAR, file)){
		char op;
		unsigned long address;
		int size = 0;
		if(line[0] == 'I'){
			continue;
		}
		else{
			sscanf(line, " %c %lx,%d", &op, &address, &size);
			if(v) printf("%c %lx,%d", op, address, size);
			if(op == 'L')
				visit(address, cache);
			else if(op == 'S')
				visit(address, cache);
			else if(op == 'M'){
				visit(address, cache);
				visit(address, cache);
			}
			else{
				printf("error!\n");
				exit(-1);
			}
			if(v) printf("\n");
		}
		for(int i = 0; i < S * E; i++){
			cache[i].lru++;
		}
	}
	
	// for(int i = 0; i < S * E; i++){
	// 	free(cache[i].data);
	// }
	free(cache);
	fclose(file);
    printSummary(hit, miss, eviction);
    return 0;
}

