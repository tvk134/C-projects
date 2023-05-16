#include <stdio.h>
#include <string.h>
#include "mymalloc.h"

//Length of our memory. Note: this value should be 4 less than the total number of bytes intended to be available.
#define MEMSIZE 4096        

typedef struct metaData{
    // 1 = memory is in use, 0 = memory is not in use
    unsigned short valid; // 2 bytes
    // [0 - 65,535]
    unsigned short chunkSize; // 2 bytes
}metaData;

//our memory - storage
static char memory[MEMSIZE + sizeof(metaData)];

static char *requestChunk(size_t size){

    char *ptr = memory;
    int index = 0;
    int last = MEMSIZE;
    metaData *header = (metaData *)ptr;

    while ( index <= last ){
        //check if header's chunksize is large enough for memory request
        if( (header->valid == 0) && (header->chunkSize >= size) ){
            return ptr;
        }
    //iterate ptr
    ptr += (header->chunkSize + sizeof(metaData));
    //update index 
    index += header->chunkSize + (sizeof(metaData));
    //update header
    header = (metaData *)ptr;
    }
    //never found a valid header/available chunk, so we return NULL
    return NULL;
}

static int checkValid(metaData* data){
    metaData* currentChunk = (metaData*)memory;
    char* current = (char*)memory+4;

    int index = 0;
    while(index<=MEMSIZE){
        if(data==currentChunk)
            return 1;
        current += currentChunk->chunkSize + 4;
        index+=currentChunk->chunkSize+4;
        currentChunk = (metaData*) (current-4);
    }
    return 0;
}


static void coalesce(){
    int index = 0;
    metaData* currentChunkHeader = (metaData*)memory;
    char* currentChunk = (char*)memory + 4;
    
    char* nextChunk = (char*)(memory + 4 + currentChunkHeader->chunkSize + 4) ;
    metaData* nextChunkHeader = (metaData*)(nextChunk - 4);

    while(index<=MEMSIZE){
        
        if(index+currentChunkHeader->chunkSize<MEMSIZE){
            if(currentChunkHeader->valid==0&&nextChunkHeader->valid==0){
                currentChunkHeader->chunkSize+=4+nextChunkHeader->chunkSize;
                //printf("\n");
                //printf("Index:%d\n",index);
                //printf("\n");
                nextChunk+=nextChunkHeader->chunkSize+4;
                nextChunkHeader = (metaData*)(nextChunk-4);
            }

            else{
            currentChunk += currentChunkHeader->chunkSize+4;
            index+=currentChunkHeader->chunkSize+4;
            currentChunkHeader = (metaData*)(currentChunk-4);
            nextChunk+=nextChunkHeader->chunkSize+4;
            nextChunkHeader = (metaData*)(nextChunk-4);
            }
        }
    
        else{
            currentChunk += currentChunkHeader->chunkSize+4;
            index+=currentChunkHeader->chunkSize+4;
            currentChunkHeader = (metaData*)(currentChunk-4);
        }        
    }


    }


//Function is meant for whitebox testing purposes

void iterator(){                                        
    metaData* currentChunk = (metaData*)memory;
    char* current = (char*)memory+4;

    int index = 0;
    while(index<=MEMSIZE){
        printf("Index:%d Chunksize:%d Valid:%d\n",index, currentChunk->chunkSize, currentChunk->valid);
        current += currentChunk->chunkSize + 4;
        index+=currentChunk->chunkSize+4;
        currentChunk = (metaData*) (current-4);
    }
    puts("");
}

static int checker(){

    for(int i = 0; i < sizeof(metaData); i++){
        if(memory[i]!= 0) return 1;
    }

    return 0;
}

void *mymalloc(size_t size, char *file, int line){

    /*check if its intialized*/
    if (checker() == 0) {    
    //pointer to our memory
    char *mem = memory;
    //pointer to our metadata
    metaData *header = (metaData *)mem; 

    //setting up our memory with a start header
    header->valid = 0;
    header->chunkSize = (MEMSIZE);
    }

    /* check for invalid size */
    if(size <= 0){
        //ERROR MESSAGE 
        printf("Error in file %s, on line %d, Cannot allocate for this size, request a size [0-%d].\n",file,line,MEMSIZE);
        return NULL;
    }

    /* larger than our memory, our metadata is already being considered since we added 4 bytes to our memory for the starter header */
    if(size > MEMSIZE){
        //ERROR MESSAGE 
        printf("Error in file %s, on line %d, Cannot allocate for this size, request a size [0-%d].\n",file,line,MEMSIZE);
        return NULL;
    }

    /* Request a chunk */
    char *req = requestChunk(size);
    /* Chunk is not available */
    if(req == NULL){
        //ERROR MESSAGE
        printf("Error in file %s, on line %d, Insufficient memory available.\n",file,line);
        return NULL;
    }

    /* Chunk is avaliable */
    metaData *header = (metaData *)req;
        
    //Making a new header, if possible (basically "splitting" our chunk if we are able to)
    // smallest region that can possible be allocated is 1 byte region, the 4 bytes for the header is already accounted for when acquiring the variable "leftover"
    int leftover = (header->chunkSize - size) - sizeof(metaData);
    
    if ( leftover >= 1 ){
        //create a new header for the remaining chunk 
        int increment = (size + sizeof(metaData));
        metaData *newHeader = (metaData *)(req + increment);
        newHeader->valid = 0;
        newHeader->chunkSize = leftover;

        //header being return will be assigned to the size client request since a new header was assigned to the remaining chunk
        header->chunkSize = size;
        header->valid = 1;

        //returning chunk address not header address
        return (req+sizeof(metaData));
    }

    //If new header wasn't created, chunk size was not large enough to be splited so we do not change the size of the header and return chunk address to client
    header->valid = 1;

    //returning chunk address not header address
    return (req+sizeof(metaData));
}

void myfree(void *ptr, char *file, int line){
    //iterator();
    if(ptr == NULL){
        printf("Error in file %s on line %d. Trying to free a NULL Pointer.\n",file,line);
        return;
    }

    if((char*)ptr<memory||(char*)ptr>(memory+MEMSIZE+4)){
        printf("Error in file %s on line %d. This address was not obtained from malloc.\n",file,line);
        return;
    }

    ptr = (char*) ptr;
    ptr = ptr-4;
    metaData* data = (metaData*) ptr;
    //point to header
    //error detection
    if(checkValid(data)==0){
        printf("Error in file %s on line %d. Invalid pointer, this address does not point to the beginning of the memory region returned by malloc.\n",file,line);
        return;
    }
    if(data->valid==0){
        printf("Error in file %s on line %d. Allocation associated with this address is already freed.\n",file, line);
        return;
    }
    
    data->valid = 0;
    if(data->chunkSize!=MEMSIZE)
    {
        coalesce(); //no need to coalesce if there's only one chunk.
    }
    //iterator();
}