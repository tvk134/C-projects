#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct node{
    char val[256];
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

void addEdge(node* head, char added[256]){
    if(head->next==NULL){
        node *add=malloc(sizeof(node));
        add->next=NULL;
        strcpy(add->val,added);
        head->next=add;
        return;
    }
    else{
        while(head->next!=NULL){
            if(strcmp((head->next->val),added)>0)
                break;
            head = head->next;
        }
        if(head->next==NULL){
            node *add=malloc(sizeof(node));
            add->next=NULL;
            strcpy(add->val,added);
            head->next=add;
        }
        else{
            node *add = malloc(sizeof(node));
            add->next=head->next;
            strcpy(add->val,added);
            head->next=add;
        }
    }
}

int retDegree(node* head){
    int degree = 0;
    while(head->next!=NULL){
        degree++;
        head = head->next;
    }
    return degree;
}

void printList(node* head){
    node* ptr = head;
    while(ptr!=NULL){
        printf("%s ",ptr->val);
        ptr = ptr->next;
    }
    printf("\n");
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

void bfs(char start[256],node** arr,int num){
    node* queue = malloc(sizeof(node));
    strcpy(queue->val,start);
    queue->next=NULL;

    bool *visited = malloc(sizeof(bool)*num);
    for(int i = 0;i<num;i++){
        if(strcmp(arr[i]->val,start)==0)
            visited[i] = true;
        else
            visited[i] = false;
    }
    
    for(int j = 0;j<num;j++){                                   //enqueues nodes that are immediately adjacent to source.
        if(strcmp(start,arr[j]->val)==0){
            node *ptr = arr[j]->next;
            while(ptr!=NULL){
                enqueue(queue,ptr->val);
                for(int i = 0;i<num;i++){
                        if(strcmp(arr[i]->val,ptr->val)==0){
                            visited[i] = 1;
                        }  
                    }
                ptr = ptr->next;
            }
        }
    }

    node *point2ElectriBoogaloo = queue->next;

    while(point2ElectriBoogaloo!=NULL){
        for(int k = 0;k<num;k++){
            if(strcmp(point2ElectriBoogaloo->val,arr[k]->val)==0){
                visited[k]=1;
                node *ptr = arr[k]->next;
                while(ptr!=NULL){
                    for(int i = 0;i<num;i++){
                        if(strcmp(arr[i]->val,ptr->val)==0){
                            if(visited[i]==0){
                                enqueue(queue,ptr->val);
                                visited[i] = 1;   
                            }
                            break;
                        }  
                    }
                    ptr = ptr->next;
                }
            }
        }
        point2ElectriBoogaloo=point2ElectriBoogaloo->next;
    }

    free(visited);
    printList(queue);
    freeNodes(queue);
}

int main(int argc, char const *argv[])
{ 
    if(argc<3){
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

    char edgeToAdd[256];

    while(fscanf(fp,"%s %s\n",vertex,edgeToAdd)!=EOF){
        for(int i = 0;i<numberOfVertexes;i++){
            if(strcmp(arr[i]->val,vertex)==0){
                addEdge(arr[i],edgeToAdd);
            }
            if(strcmp(arr[i]->val,edgeToAdd)==0){
                addEdge(arr[i],vertex);
            }
        }
    }

    FILE* query = fopen(argv[2],"r");

    while(fscanf(query,"%s\n",vertex)!=EOF)
    {
        bfs(vertex, arr, numberOfVertexes);
    }


    for(int l = 0;l<numberOfVertexes;l++){
        freeNodes(arr[l]);
    }

    fclose(fp);
    free(arr);
    return 0;
}
