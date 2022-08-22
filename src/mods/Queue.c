#include <stdlib.h>
#include <stdio.h>
#include "../libs/Queue.h"

//Queue is created with a List struct , we remove from the first node of list and add to the end of the list
//Queue is used by the manager , to save all stopped workers and to save all worker pids

typedef struct lnode{           //define struct for node of list

    void* item;
    Lnode* next;

}Lnode;

typedef struct list{           //define struct List

    Lnode* first;
    Lnode* last;
    int size;

}List;

List* list_create(){          //function returns pointer to empty struct List*

    List* list = malloc(sizeof(*list));
    list->size = 0;
    list->first = NULL;
    list->last = NULL;
    return list;

}

void list_insert_last(List* list , void* temp){         //insert node with value void* temp at the end of the list

    Lnode* node = malloc(sizeof(*node));
    node->item = temp;

    if(list->first == NULL){

        list->first = node;
        list->last = node;
        node->next = NULL;
        list->size++;
    }

    else if((list->first == list->last) && (list->first != NULL) && (list->last != NULL)){

        list->first->next = node;
        list->last = node;
        node->next = NULL;
        list->size++;

    }

    else{

        list->last->next = node;
        list->last = node;
        node->next = NULL;
        list->size++;

    }

}

void* list_remove_first(List* list){            //remove first node of list and return value of node (void* temp)

    if(list->first->next == NULL){

        Lnode* node_to_remove = list->first;
        void* temp = node_to_remove->item; 
        free(node_to_remove);
        list->first = NULL;
        list->last = NULL;
        list->size--;
        return temp;        

    }

    Lnode* node_to_remove = list->first;
    void* temp = node_to_remove->item; 
    list->first = node_to_remove->next;
    free(node_to_remove);
    list->size--;
    return temp;
}

void print_list(List* list){                                        //prints entire list

    for(Lnode* node = list->first;node != NULL;node = node->next){
        int* temp = node->item;
        printf("%d \n",*temp);

    }

}

void list_free(List* list){               //frees all nodes inside list , then frees list as well

    if(list->first != list->last){

        for(Lnode* node = list->first;node != NULL;node = node->next){

            free(node);

        }

    }

    else{

        free(list->first);

    }

    free(list);

}


typedef struct queue{               //define struct queue , only variable is a struct pointer List*

    List* List;

}Queue;

Queue* queue_create(){          //create a Queue* queue with malloc and return it

    Queue* queue = malloc(sizeof(*queue));
    queue->List = list_create();
    return queue;

}

void queue_insert(Queue* queue, void* temp){        //insert value void* temp in queue

    list_insert_last(queue->List,temp);

}

int queue_size(Queue* queue){                   //return size of Queue(size of List)

    return queue->List->size;

}

void* queue_delete(Queue* queue){               //remove first element of queue(first element of list)

    void* temp = list_remove_first(queue->List);
    return temp;

}

int queue_empty(Queue* queue){              //return 1 if queue is empty , else return 0

    if(queue->List->size == 0){

        return 1;

    }

    else{

        return 0;

    }

}

void queue_free(Queue* queue){          //call list_free to free the List* list , then free queue

    list_free(queue->List);
    free(queue);

}