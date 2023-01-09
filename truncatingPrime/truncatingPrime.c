#include <stdio.h>
#include <stdlib.h>

int checker(int a){
    if(a<1)
        return 0;
    
    while(a!=0)
    {
        for(int i = 2;i<=(a/2);i++){
            if(a%i==0)
                return 0;
        }
        a=a/10;
    }
    return 1;
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
    
    int lines;
    fscanf(fp,"%d\n",&lines);

    while(lines!=0){
        
        int num;
        fscanf(fp,"%d\n",&num);

        if(checker(num) == 0)
            printf("no\n");
        else
            printf("yes\n");

        lines--;
    }
    fclose(fp);
    return EXIT_SUCCESS;
}
