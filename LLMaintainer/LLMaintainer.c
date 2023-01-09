#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct node{
    int value;
    struct node *link;
};

void freeAll(struct node *head){
    struct node *ptr = head;

    while(head!=NULL){
        head = head->link;
        free(ptr);
        ptr=head;
    }
}

void printOut(struct node *head){
    while(head!=NULL){
        printf("%d ",head->value);
        head = head->link;
    }
    printf("\n");
}

void insert(struct node **head, int val){
    struct node *current = *head;

    if(current==NULL){
        current = malloc(sizeof(struct node));
        current->value=val;
        current->link=NULL;
        *head = current;
        return;
    }

    if(current->value==val)
        return;

    struct node *ins = malloc(sizeof(struct node));
    ins->value=val;

    if(head==NULL){
        *head=ins;
    }
    
    if(current->value>val){
        ins->link=current;
        *head = ins;
        return;
    }

    struct node *prev = current;
    current = current->link;
    while(current!=NULL){

        if(current->value==val){
            free(ins);
            return;
        }

        if(current->value>val)
        {
            prev->link=ins;
            ins->link=current;
            return;
        }

        prev = current;
        current = current->link;
    }

    if(current==NULL){
        prev->link=ins;
        ins->link=NULL;
    }
}

void delete(struct node **head, int val){
    struct node *current = *head;
    if(current!=NULL){
        if(current->value==val)
        {
            *head = current->link;
            free(current);
            return;
        }
        struct node *prev = current;
        current = current->link;
        while(current!=NULL){
            if(current->value==val)
            {
                if(current==*head)
                {
                    head=NULL;
                    return;
                }
                prev->link=prev->link->link;
                free(current);
                return;
            }
            prev = current;
            current = current->link;
        }
    }
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

    char func[256];
    int num;

    struct node *head = malloc(sizeof(struct node));
    fscanf(fp,"%s %d\n",func,&num);
    while(strcmp(func,"DELETE")==0)
    {
        printf("EMPTY\n");
        fscanf(fp,"%s %d\n",func,&num);
    }

    head->value=num;
    head->link=NULL;
    printOut(head);

    while(fscanf(fp,"%s %d \n",func,&num)!=EOF)
    {   
        if(strcmp(func,"DELETE")==0){
            delete(&head,num);
        }

        else{
            insert(&head,num); 
        }

        if(head==NULL)
            printf("EMPTY\n");

        else
            printOut(head);

    }

    freeAll(head);
    fclose(fp);
    return 0;
}

