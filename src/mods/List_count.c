#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../libs/List_count.h"

//List_count is used by the worker to save all locations found , along with the numbrt of times they appeared

typedef struct lnode_count{             //struct for List_count nodes

    char* location;
    int count;
    LnodeC* next;

}LnodeC;

char* node_location(LnodeC* node){      //returns location of LnodeC* node

    return node->location;

}

int node_count(LnodeC* node){           //returns count of LnodeC* node

    return node->count;

}

typedef struct list_count{              //struct of List_count

    LnodeC* first;
    LnodeC* last;
    int size;

}ListC;


ListC* list_count_create(){             //creates ListC* pointer and returns it

    ListC* list_count = malloc(sizeof(*list_count));
    list_count->size = 0;
    list_count->first = NULL;
    list_count->last = NULL;
    return list_count;

}

void list_count_insert(ListC* list_count,char* location){               //inserts char* location to the list , if location is already in list we increase count by 1

    int flag = 0;                                                       //otherwise create a new node and add it to the end of the list

    for(LnodeC* temp = list_count->first;temp != NULL;temp = temp->next){

        if(strcmp(location,temp->location) == 0){

            temp->count++;
            flag = 1;
            break;
        }

    }

    if((list_count->first == NULL) && (flag == 0)){

        LnodeC* node = malloc(sizeof(*node));
        node->location = malloc((strlen(location) + 1) * sizeof(char));
        strcpy(node->location,location);
        node->count = 1;
        node->next = NULL;
        list_count->first = node;
        list_count->last = node;
        list_count->size++;

    }

    else if((list_count->first == list_count->last) && (list_count->first != NULL) && (flag == 0)){

        LnodeC* node = malloc(sizeof(*node));
        node->location = malloc((strlen(location) + 1) * sizeof(char));
        strcpy(node->location,location);
        node->count = 1;
        node->next = NULL;
        list_count->first->next = node;
        list_count->last = node;
        list_count->size++;

    }

    else if(flag == 0){

        LnodeC* node = malloc(sizeof(*node));
        node->location = malloc((strlen(location) + 1) * sizeof(char));
        strcpy(node->location,location);
        node->count = 1;
        node->next = NULL;
        list_count->last->next = node;
        list_count->last = node;
        list_count->size++;

    }    

}

int list_count_size(ListC* list_count){             //return size of list

    return list_count->size;

}

LnodeC* list_count_remove(ListC* list_count){       //removes first node of list and returns it , if list is empty return NULL

    if(list_count->first != NULL){

        LnodeC* temp = list_count->first;
        list_count->first = temp->next;
        list_count->size--;
        return temp;

    }

    else{

        return NULL;

    }

}

void list_count_destroy(ListC* list_count){         //free struct point ListC*

    free(list_count);

}