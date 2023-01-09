#include <stdlib.h>
#include <stdio.h>

void freeAll(int** square,int num){    
    for(int i = 0;i<num;i++)
    {
        free(square[i]);
    }
}

void getSmallerMatrix(int**matrix, int**smallerMatrix, int r, int c, int size){
    int i = 0,j=0;
            for(int row = 0; row<size;row++){
                for(int col = 0;col<size;col++){
                    if(row!=r&&col!=c){
                        smallerMatrix[i][j++]=matrix[row][col];
                        if(j==size-1){
                            j=0;
                            i++;
                        }
                    }

                }
            }

}

int calcDet(int**matrix, int num){
    int det = 0;
    if(num==1)
        det = matrix[0][0];
    else if(num==2)
        det = (matrix[0][0]*matrix[1][1])-(matrix[1][0]*matrix[0][1]);

    else{
        int**smallerMatrix = malloc(sizeof(int*)*(num-1));
        for(int i = 0;i<num-1;i++){
            smallerMatrix[i] = malloc(sizeof(int)*(num-1));
        }

        int sign = 1;

        for(int a = 0;a<num;a++){     
            getSmallerMatrix(matrix,smallerMatrix, 0, a, num);
            det+=sign*matrix[0][a]*calcDet(smallerMatrix,num-1);
            sign = -1*sign;

        }
        freeAll(smallerMatrix,num-1);
        free(smallerMatrix);

    }    

    return det;
}

int main(int argc, char const *argv[])
{
    if(argc<2){
        printf("ERROR: INSUFFICIENT ARGUMENTS\n");
        return EXIT_SUCCESS;
    }

    FILE* fp = fopen(argv[1],"r");

    if(fp==NULL){
        printf("error\n");
        return EXIT_FAILURE;
    }

    int num;

    fscanf(fp,"%d\n",&num);

    int** matrix = malloc(sizeof(int*)*num);

    for(int i = 0;i<num;i++){
        matrix[i] = malloc(sizeof(int)*num);
    }  

    for(int i = 0;i<num;i++){
        for(int j = 0;j<num;j++){
            int n = 0;
            fscanf(fp,"%d",&n);
            matrix[i][j] = n;
        }
    }

    printf("%d\n",calcDet(matrix,num));
    calcDet(matrix,num);
    freeAll(matrix,num);
    free(matrix);
    return 0;
}
