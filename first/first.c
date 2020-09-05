#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include<math.h>

//create cacheLine struct
struct cacheLine{
	unsigned int valid;
	unsigned long int tag;
	unsigned int time;
	struct cacheLine *next;
};

//initialize output global variables
int memReads = 0;
int memWrites = 0;
int cacheHits = 0;
int cacheMisses = 0;

//print output
void printOutput(){
	printf("Memory reads: %d\n", memReads);
	printf("Memory writes: %d\n", memWrites);
	printf("Cache hits: %d\n", cacheHits);
	printf("Cache misses: %d\n",  cacheMisses);
}

//method to allocate and simulate initial cache where sets is number of sets and assoc is associativity
struct cacheLine** allocate(int sets, int assoc){
	int i;
	int j;
	struct cacheLine** temp = malloc(sizeof(struct cacheLine) * sets);
	for (i = 0; i < sets; i++)
	{
		temp[i] = malloc(sizeof(struct cacheLine) * assoc);
			for (j = 0; j < assoc; j++)
			{
				temp[i][j].valid=0;
				//in this way the cache is like a 2D array (which works in the way we want to simulate it
			}
	}
	return temp;
}

//method to read cache
void readCache(struct cacheLine** cache, unsigned long tag, int index, int sets, int assoc){
	int flag=0;
	int i;
	//check for validity of data and match of tag
	for(i=0; i < assoc; i++)
	{
		if(cache[index][i].valid==1 && cache[index][i].tag==tag)
		{
			//cache hit
			flag=1;
			cacheHits++;
			break;
		}
	}
	if(flag==1)
	{
		return;
	}
	else
	{
		cacheMisses++;
		memReads++;
		int val = 1;
        for(i=0; i<assoc; i++)
        {
        	if(cache[index][i].valid==1)
        	{
        		cache[index][i].time++;
        	}
        	if(cache[index][i].valid==0)
        	{
        		val = 0;
        		break;
        	}
        }
        int mTime=0;
        if(val == 1)
        {
        	for(i=0; i<assoc; i++)
        	{
        		if(cache[index][i].time > mTime)
        		{
        			mTime = cache[index][i].time;
        		}
        	}
        	for(i=0; i<assoc; i++)
        	{
        		if(cache[index][i].time==mTime)
        		{
        			cache[index][i].valid=0;
        			cache[index][i].tag=(-1);
        			cache[index][i].time=0;
        			break;
        		}
        	}
        }
        //set valid tags
        for(i=0; i < assoc; i++)
        {
			if(cache[index][i].valid==0)
			{
				cache[index][i].valid=1;
				cache[index][i].tag=tag;
			}
		}
	}
}

//method to write cache
void writeCache(struct cacheLine** cache, unsigned long tag, int index, int sets, int assoc){
        int flag = 0;
	int i;
        for(i = 0; i < assoc; i++)
        {
        	//cache hit
        	if(cache[index][i].valid==1 && cache[index][i].tag == tag)
        	{
        		flag = 1;
        		cacheHits++;
        		memWrites++;
        		break;
        	}
        }
        if(flag==1)
        {
                return;
        }
        //cache miss
        else
        {
	        readCache(cache, tag, index, sets, assoc);
	        memWrites++;
	        return;
        }
}

//method to find n of 2^n where x=2^n
int logTwo(int x){
	if(x==0)
	{
		return 0;
	}
	while(x!=1)
	{
		if(x%2!=0)
		{
			return 0;
		}
		x=x/2;
	}
	return 1;
}

int main(int argc, char** argv){
	//check for correct number of args
	if (argc != 6)
	{
		printf("Invalid Quantity of Args Error\n");
		exit(0);
	}
	//open file
	FILE* fptr = fopen(argv[5], "r");
  	if(fptr == NULL)
  	{
  		printf("No Trace File Error\n");
  		exit(0);
  	}
  	//initialize useful variables
	int cacheSize = atoi(argv[1]);
	int blockSize = atoi(argv[2]);
	int asc = 0;
	int numSets = 0;
	//check for valid cache size and block size
	if(logTwo(cacheSize)==0 || logTwo(blockSize==0))
	{
		printf("Invalid Cache Error\n");
		return 0;
	}
	//check input type of associativity
	if(strcmp(argv[4], "direct")==0)
	{
		asc = 1;
		numSets = cacheSize/blockSize;
	}
	else if(strcmp(argv[4], "assoc")==0)
	{
		asc = cacheSize/blockSize;
		numSets = 1;
	}
	else if(strncmp(argv[4], "assoc:", 6)==0)
	{
		sscanf(argv[4], "assoc:%d", &asc);
		if(logTwo(asc)==0)
		{
			printf("Invalid n-Way Associativity\n");
			return 0;
		}
		numSets = cacheSize/(blockSize*asc);
	}
	else
	{
		printf("Invalid Associativity Error\n");
		return 0;
	}
	//if there are no errors up to this point, the input arguments are all valid. we can ignore argv[3] since we are not doing the extra credit for now
	//allocate cache
	struct cacheLine** tCache = allocate(numSets, asc);
	//initialize variables for address and function type
	unsigned long int address = 0;
	char func = ' ';
	//calculate bits for offset and set numbers using logs
	int offsetBits = (int) (log(blockSize)/log(2));
	int setBits = (int) (log(numSets)/log(2));
	//use bit shifting and masking (indexHelper var) to get index address
	int indexHelper = (1 << setBits) - 1;
	//scan the file and perform operations
	while((fscanf(fptr, "%c %lx\n", &func, &address)!=EOF && func!='#'))
	{
		unsigned int ind = (address >> offsetBits) & indexHelper;
		unsigned long tag = (address >> offsetBits) >> setBits;
		if(func=='R')
		{
			readCache(tCache, tag, ind, numSets, asc);
		}
		else if(func=='W')
		{
			writeCache(tCache, tag, ind, numSets, asc);
		}
	}
	fclose(fptr);
	//print output for grading
	printOutput();

	return 0;
}
