#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

typedef struct block{
    bool valid;
    int age;
    unsigned long tag;
}block;

typedef struct set{
    block *contents;
}set;

int hitOrMiss(set toCheck, int size, unsigned long tag){
    for(int i = 0;i<size;i++){
        if(toCheck.contents[i].valid&&toCheck.contents[i].tag==tag)
            return i;
    }
    
    return -1;
}

int emptyOrEvictBlock(set toCheck, int size){
    int index = -1;
    for(int i = 0;i<size;i++){
        if(!toCheck.contents[i].valid){
            return i;
        }            
    }

    if(index==-1){
        index = 0;
        for(int j = 0;j<size;j++){
            if(toCheck.contents[j].age<toCheck.contents[index].age){
                index = j;
            }
        }
    }

    return index;
}

int main(int argc, char const *argv[])
{
    if(argc<5){
        printf("ERROR: INSUFFICIENT ARGUMENTS\n");
        return EXIT_SUCCESS;
    }

    int cacheSize = atoi(argv[1]);
    int assoc = 0;
    sscanf(argv[2],"assoc:%d",&assoc);
    int blockSize = atoi(argv[4]);

    bool policy;
    if(strcmp(argv[3],"fifo")==0){
        policy = true;
    }
    else policy = false;

    int numberOfSets = cacheSize/(blockSize*assoc);
    set *cache = malloc(sizeof(set)*numberOfSets);

    for(int i = 0;i<numberOfSets;i++){
        cache[i].contents=malloc(sizeof(block)*assoc);
        for(int s = 0;s<assoc;s++){
            cache[i].contents[s].age = 0;
            cache[i].contents[s].valid = false;
            cache[i].contents[s].tag = 0;
        }
    }

    FILE* fp = fopen(argv[5],"r");    
    if(fp==NULL){
        printf("error\n");
        return EXIT_FAILURE;
    }
    
    char func;
    unsigned long address;

    int memreads = 0;
    int memwrites = 0;
    int cachehits = 0;
    int cachemisses = 0;
    int globalAge = 0;

    int blockOffsetBits = log2(blockSize);
    //int indexBits = log2(assoc);

    while(fscanf(fp,"%c %lx\n",&func, &address)!=EOF){
        int setOffset = log2(numberOfSets);
        unsigned long setIndex = (address>>blockOffsetBits) & ((1<<setOffset)-1);
        unsigned long tag = (address>>(blockOffsetBits+setOffset));

        if(func=='W'){
            memwrites++;
        }

        int blockIndex = hitOrMiss(cache[setIndex],assoc,tag);
        
        if(blockIndex!=-1){
            cachehits++;
            if(!policy){
                cache[setIndex].contents[blockIndex].age=globalAge;
            }
        }

        else if(blockIndex==-1){
            cachemisses++;
            blockIndex = emptyOrEvictBlock(cache[setIndex],assoc);
            memreads++;

            cache[setIndex].contents[blockIndex].tag=tag;
            cache[setIndex].contents[blockIndex].valid=true;
            cache[setIndex].contents[blockIndex].age=globalAge;
        }
        globalAge++;
    }

    printf("memread:%d\n",memreads);
    printf("memwrite:%d\n",memwrites);
    printf("cachehit:%d\n",cachehits);
    printf("cachemiss:%d\n",cachemisses);

    for(int j = 0;j<numberOfSets;j++){
        free(cache[j].contents);
    }
    free(cache);
    fclose(fp); 
    return EXIT_SUCCESS;
}