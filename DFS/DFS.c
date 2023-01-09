#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct node{
    char val[256];
    int weight;
    struct node* next;
} node;

void freeNodes(node* head){
    node *ptr = head;
    while(head!=NULL){
        head = head->next;
        free(ptr);
        ptr=head;
    }
}

void addEdge(node* head, char added[256],int weight){
    if(head->next==NULL){
        node *add=malloc(sizeof(node));
        add->next=NULL;
        strcpy(add->val, added);
        add->weight=weight;
        head->next=add;
        return;
    }
    else{
        while(head->next!=NULL){
            if(strcmp(head->next->val,added)>0)
                break;
            head = head->next;
        }
        if(head->next==NULL){
            node *add=malloc(sizeof(node));
            add->next=NULL;
            strcpy(add->val, added);
            add->weight=weight;
            head->next=add;
        }
        else{
            node *add = malloc(sizeof(node));
            add->next=head->next;
            strcpy(add->val, added);
            add->weight=weight;
            head->next=add;
        }
    }
}

int retOutDegree(node* head){
    int degree = 0;
    while(head->next!=NULL){
        degree++;
        head = head->next;
    }
    return degree;
}

int retInDegree(node** arr, int size, char target[256]){
    int degree = 0;

    for(int i = 0;i<size;i++){
        node* head = arr[i];
        if(strcmp(head->val,target)!=0){
                while(head!=NULL){
                if(strcmp(head->val,target)==0)
                    degree++;
                head = head->next;
            }
        }
    }

    return degree;
}

void sortArr(node** arr,int size){
    char temp[256];
    for(int i = 0;i<size;i++){
        for(int j = i+1;j<size;j++){
            if(strcmp(arr[i]->val,arr[j]->val)>0){
                strcpy(temp,arr[i]->val);
                strcpy(arr[i]->val,arr[j]->val);
                strcpy(arr[j]->val,temp);
            }
        }
    }
}

void enqueue(node* list, char val[256]){
    while(list->next!=NULL){
        list = list->next;
    }
    node* add = malloc(sizeof(node));
    strcpy(add->val,val);
    add->next=NULL;
    list->next=add;
}

void printList(node* head){
    while(head!=NULL){
        printf("%s ",head->val);
        head = head->next;
    }
    printf("\n");
}

void dfs(node** arr, int num, bool* visited, char start[256]){
    int index;
    for(index=0;index<num;index++){
        if(strcmp(arr[index]->val,start)==0){
            break;
        }
    }

    if(!visited[index]){
        visited[index] = true;
        printf("%s ",arr[index]->val);
        node *ptr = arr[index]->next;
        while(ptr!=NULL){
            dfs(arr, num, visited, ptr->val);
            ptr = ptr->next;
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

    int numberOfVertexes=0;
    fscanf(fp,"%d\n",&numberOfVertexes);

    node** arr = malloc(sizeof(node*)*numberOfVertexes);

    char vertex[256];

    for(int i = 0;i<numberOfVertexes;i++){
        fscanf(fp,"%s\n",vertex);
        arr[i] = malloc(sizeof(node));
        arr[i]->next=NULL;
        strcpy(arr[i]->val,vertex);
    }

    sortArr(arr,numberOfVertexes);

    char edgeToAdd[256];
    int weight=0; 
    while(fscanf(fp,"%s %s %d\n",vertex,edgeToAdd,&weight)!=EOF){
        for(int i = 0;i<numberOfVertexes;i++){
            if(strcmp(arr[i]->val,vertex)==0){
                addEdge(arr[i],edgeToAdd,weight);
            }
        }
    }

    bool *visited = malloc(sizeof(bool)*numberOfVertexes);
    dfs(arr, numberOfVertexes,visited,arr[0]->val);

    for(int i = 0;i<numberOfVertexes;i++){
        if(!visited[i]){
            dfs(arr,numberOfVertexes,visited,arr[i]->val);
        }
    }

    printf("\n");

    for(int l = 0;l<numberOfVertexes;l++){
        freeNodes(arr[l]);
    }

    fclose(fp);
    free(visited);
    free(arr);
    return 0;
}
