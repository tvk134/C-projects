#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include "mymalloc.h"



double test1(){
    int iteration = 0;
    double test1 = 0;
    struct timeval current_time;
    while(iteration<50){
    gettimeofday(&current_time,NULL);
    long timeBefore = current_time.tv_usec;
    //Test 1: allocating and immediately freeing 1 byte chunk 120 times
    for(int i = 0;i<120;i++){
        free(malloc(1));        //no need to store allocated chunk in a pointer since we aren't doing anything to it. Freed immediately.
                                //Note: Although chunk size is only 1 byte, each allocated chunk will always take up 5 bytes since each header requires 4.
    }

    gettimeofday(&current_time,NULL);
    test1+=(current_time.tv_usec-timeBefore);
    iteration++;
    }
    return test1;
}

double test2(){
    int iteration = 0;
    double test2 = 0;
    struct timeval current_time;
    while(iteration<50){
    //Test 2: allocate 120 chunks, store pointers in an array, and then deallocate.
        gettimeofday(&current_time,NULL);
        long timeBefore = current_time.tv_usec;
        char* arr1[120];
        for(int j = 0;j<120;j++){
            arr1[j] = malloc(1);
        }
        for(int k = 0;k<120;k++){
            free(arr1[k]);
        }

        gettimeofday(&current_time,NULL);
        test2+=(current_time.tv_usec-timeBefore);
        iteration++;
    }
    return test2;
}

double test3(){
    int iteration = 0;
    double test3 = 0; 
    struct timeval current_time;
    char* arr2[120];
    time_t t;
    srand((unsigned)time(&t));
    
    while(iteration<50){
        //Test 3:
        gettimeofday(&current_time,NULL);
        long timeBefore = current_time.tv_usec;
        // stop when malNum = 120, freeNum = 0
        // Null pointers
        for(int i = 0;i<120;i++){
            arr2[i] = NULL;
        }
        int numMallocs = 0;
        while(numMallocs<120){
            int func = rand()/((RAND_MAX)/2);
            if(func==0)
            {
                //malloc
                for(int i = 0;i<120;i++){
                    if(arr2[i]==NULL){
                        arr2[i] = malloc(1);
                        numMallocs++;
                        break;
                    }
                }
            }
            else{
                for(int j = 0;j<120;j++){
                    if(arr2[j]!=NULL){
                        free(arr2[j]);
                        arr2[j] = NULL;
                        break;
                    }
                }
            }
        }
        gettimeofday(&current_time,NULL);
        test3+=(current_time.tv_usec-timeBefore);
        iteration++;
        for(int k = 0;k<120;k++){
            if(arr2[k]!=NULL){
                free(arr2[k]);
                arr2[k] = NULL;
            }
        }
    }
    return test3;
}

double test41(){
    int iteration = 0;
    double test41 = 0; 
    struct timeval current_time;
    while(iteration<50){
    //Test 4.1: Allocating, filling up, and deallocating a 2-Dimensional integer array and then a 3-Dimensional integer array
        gettimeofday(&current_time,NULL);
        long timeBefore = current_time.tv_usec;
        int arrSize = 9;
        int** testArr = malloc(arrSize*sizeof(int*));
        for(int l = 0;l<arrSize;l++){
            testArr[l] = malloc(arrSize*sizeof(int));
            for(int m = 0;m<arrSize;m++){
                testArr[l][m] = l+m;
            }
        }

        //freeing contents of 2D array
        for(int out = 0;out<arrSize;out++){
            free(testArr[out]);
        }
        free(testArr);

        int*** threeDims = malloc(arrSize*sizeof(int**));

        for(int i = 0;i<arrSize;i++){
            threeDims[i] = malloc(arrSize*sizeof(int*));
            for(int j = 0;j<arrSize;j++){
                threeDims[i][j] = malloc(arrSize*sizeof(int));
                for(int k = 0;k<arrSize;k++){
                    threeDims[i][j][k] = i+j+k;
                }
            }
        }

        //free 3d array
        for(int x = 0;x<arrSize;x++){
            for(int y = 0; y<arrSize;y++){
                free(threeDims[x][y]);
            }
            free(threeDims[x]);
        }

        free(threeDims);

        gettimeofday(&current_time,NULL);
        long timeAfter = current_time.tv_usec;
        test41+=(timeAfter-timeBefore);
        iteration++;    
    }
    return test41;
}

double test42(){
    int iteration = 0;
    double test42 = 0; 
    struct timeval current_time;
    while(iteration<50){
        gettimeofday(&current_time,NULL);

        long timeBefore = current_time.tv_usec;
    
        int index = 0;
        char *arr3[820];
        //first allocation, meaning we have 819 allocations left
        arr3[index] = malloc(1);
        //fill memory up entirely with 1 byte chunk
        while(index < 820){
            //need to increment first, so we dont write over the pointer at index 0 before the loop
            index++;
            //we don't want to write a pointer at arr3[820] since that does not exist, only index 0-819
            if(index == 820) {break;}
            arr3[index] = malloc(1);
        }
        // freeing odd pointers, then even pointers, only freeing odd pointers doesn't require coalesce to do anything, when we start freeing the even pointers thats when coalesce starts merge blocks
        for(int j = 0; j < index; j=j+2){
        free(arr3[j]);
        }
        for(int c = 1; c < index; c=c+2){
        free(arr3[c]);
        }

        
        gettimeofday(&current_time,NULL);
        long timeAfter = current_time.tv_usec;
        test42+=(timeAfter-timeBefore);
        iteration++;
    }
    return test42;

}

int main (int argc, char** argv){

    printf("Test 1 takes an average of: %0.3f microseconds\n",test1()/50);
    printf("Test 2 takes an average of: %0.3f microseconds\n",test2()/50);
    printf("Test 3 takes an average of: %0.3f microseconds\n",test3()/50);
    printf("Test 4.1 takes an average of: %0.3f microseconds\n",test41()/50);
    printf("Test 4.2 takes an average of: %0.3f microseconds\n",test42()/50);

    return 0;
} 