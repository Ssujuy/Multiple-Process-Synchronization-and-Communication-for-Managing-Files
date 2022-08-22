#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "libs/List_count.h"     

#define DIRECTORY "results/"                    //directory path for .out files to be saved
#define FIFO "/tmp/worker"
#define BUFFSIZE 100

void exit_handler(int signum);

char* dynamic_buff;                             //global dynamic variables for exit_handler to free
char* fifo_name;
ListC* list;

int main(){

    pid_t pid;                                  //variables we will use later
    int filedes1,filedes2,count = 0,index = 0,flag1 = 0,flag2 = 0;
    char check_url[11];
    int buff_count = 1;
    int readfd;
    int n;
    char cc[20];
    char buff[BUFFSIZE];
    dynamic_buff = malloc(BUFFSIZE * buff_count * sizeof(char));

    list= list_count_create();                                      //create List_count to save location and times it appeared
    LnodeC* node;
    pid = getpid();
    sprintf(cc,"%d",pid);
    fifo_name = malloc((strlen(FIFO) + strlen(cc) + 1) * sizeof(char));             //create fifo path to communicate with manager
    strcpy(fifo_name,FIFO);
    strcat(fifo_name,cc);

    static struct sigaction act2;                                   //define struct sigaction act2 to catch SIGINT signal
    act2.sa_handler = exit_handler;
    sigemptyset(&(act2.sa_mask));
    sigaction(SIGINT,&act2,NULL);                                   //catching SIGINT signal sent from parent process(manager)

    if((readfd = open(fifo_name,O_RDONLY,0)) < 0){                  //open fifo , with read only flags

        perror("FIFO open");
        exit(1);

    }

    while((n = read(readfd,buff,sizeof(buff))) > 0){                //read inside while loop , we basically read path of file that cause the event 

        if(n == BUFFSIZE){                                           //if size of bytes read was 100(BUFFSIZE) 

            strcat(dynamic_buff,buff);
            buff_count++;
            dynamic_buff = realloc(dynamic_buff,buff_count * BUFFSIZE * sizeof(char));          //use strcat to put the event in dynamic_buf , realloc and read again
            memset(buff,0,sizeof(buff));

        }

        else{

            memset(dynamic_buff,0,strlen(dynamic_buff));              //make dynamic_buff empty
            strcat(dynamic_buff,buff);
            int length = strlen(dynamic_buff);


            for(int a = 0;a < length;a++){                            //because event from i notifywait ends in \n , we have to get rid of it to get the path

                if(dynamic_buff[a] == '\n'){

                    dynamic_buff[a] = '\0';

                }

            }

            char* temp_buff = malloc((strlen(dynamic_buff) + 1) * sizeof(char));
            strcpy(temp_buff,dynamic_buff);
            char* file = malloc(strlen(temp_buff));

            char* t = strtok(temp_buff,"/");

            while(1){                                               //here i use strtok to split the path and get only the name of the file without the path

                if(t == NULL){

                    break;

                }

                memset(file,0,strlen(file));
                strcpy(file,t);
                t = strtok(NULL,"/");

            }

            free(temp_buff);

            if((filedes1 = open(dynamic_buff,O_RDONLY,0)) < 0){                 //open file thaty caused the event with read only flag

                perror("open haha");
                exit(1);

            }

            char* temp = malloc(((strlen(file) + 1) + (strlen(DIRECTORY)) + (strlen(".out")))*sizeof(char));                //create .out file where worker will write all locations found
            strcpy(temp,DIRECTORY);
            strcat(temp,file);
            strcat(temp,".out");
            
            if((filedes2 = open(temp,O_RDWR | O_CREAT,0666)) == -1){                        //open .out file with  O_CREAT as flag , so it is created if it does not exist

                perror("open");
                exit(1);

            }

            free(file);
            free(temp);

            char c[2];

            while((count = read(filedes1,c,1)) > 0){                                        //read from file that cause the event , byte by byte

                if(index == BUFFSIZE){

                    buff_count++;
                    dynamic_buff = realloc(dynamic_buff,BUFFSIZE * buff_count);

                }

                if(index == 7){                                         //checks if a url was found 

                    strcpy(check_url,dynamic_buff);                     //a url will start with http://
                    check_url[7] = '\0';

                    if(strcmp(check_url,"http://") == 0){

                        flag2 = 1;

                    }

                }

                if(index == 11){

                    strcpy(check_url,dynamic_buff);
                    check_url[11] = '\0';
                    if(strcmp(check_url,"http://www.") == 0 ){          //or with http://www.

                        flag1 = 1;

                    }

                }

                if((strcmp(c,"\n") != 0) && (strcmp(c," ") != 0)){      //continue reading if string is not cut by \n or space

                    dynamic_buff[index] = c[0];
                    index++;

                    if(index < (buff_count * BUFFSIZE)){

                        dynamic_buff[index + 1] = '\0';
                    }

                }

                else{                                               //flag2 and flag1 indicate that a url was found

                    if((flag2 == 1) && (flag1 == 1)){               //here we have a url of http://www. , so we have to split it after the www.

                        dynamic_buff[index] = '\n';
                        index++;
                        int i;
                        for(i = 0;i < index;i++){

                            if(i <= 10){

                                continue;

                            }
                            
                            else if((i > 10) && ((dynamic_buff[i] == '/') || (dynamic_buff[i] == '\n') || (dynamic_buff[i] == ' '))){           //checks where the location of url ends

                                break;

                            }

                        }

                        i--;                                       //split the url and take the location
                        int location_size = i - 10;
                        char* location = malloc((location_size + 1) * sizeof(char));

                        for(int k = 11;k <= i;k++){

                            location[k - 11] = dynamic_buff[k];

                        }
                        location[location_size] = '\0';

                        for(;index >0;index--){

                            dynamic_buff[index] = '\0';

                        }
                        dynamic_buff[index] = '\0';

                        if((flag1 == 1) && (flag2 == 1)){

                            list_count_insert(list,location);           //we insert the location found to List_count
                            flag1 = 0;
                            flag2 = 0;

                        }
                        free(location);
                    }

                    else if((flag2 == 1) && (flag1 == 0)){              //here url is of type http://

                        dynamic_buff[index] = '\n';
                        index++;
                        int i;
                        for(i = 0;i < index;i++){

                            if(i <= 6){

                                continue;

                            }
                            
                            else if((i > 6) && ((dynamic_buff[i] == '/') || (dynamic_buff[i] == '\n') || (dynamic_buff[i] == ' '))){            //stop where url ends , either on space \n or first /

                                break;

                            }

                        }
                        i--;
                        int location_size = i - 6;                                  //again as before we create the location of the url
                        char* location = malloc((location_size + 1)* sizeof(char));

                        for(int k = 7;k <= i;k++){

                            location[k - 7] = dynamic_buff[k];


                        }

                        location[location_size] = '\0';

                        for(;index >0;index--){

                            dynamic_buff[index] = '\0';

                        }
                        dynamic_buff[index] = '\0';

                        if((flag1 == 0) && (flag2 == 1)){

                            flag2 = 0;
                            list_count_insert(list,location);               //and the we insert it to List_count

                        }

                        free(location);

                    }

                    else{

                        index = 0;
                        memset(dynamic_buff,0,sizeof(dynamic_buff));

                    }

                }
            }

            flag1 = 0;                              //here we empty all variables , if another file is read for another loop
            flag2 = 0;
            index = 0;
            close(filedes1);
            node = list_count_remove(list);

            while(node != NULL){                    //we have finished reading the entire file and we iterate the whole List_count list

                int count = node_count(node);

                if(count > 1){                      //count is > 1 , so we take the location along with the number that it appeared

                    char cc[10];
                    char* temp  = node_location(node);
                    sprintf(cc,"%d",count);
                    write(filedes2,temp,strlen(temp));          //we write everything to the . out file
                    write(filedes2,"\t",strlen("\t"));
                    write(filedes2,cc,strlen(cc));
                    write(filedes2,"\n",strlen("\n"));
                    free(temp);
                    if(node != NULL){

                        free(node);

                    }

                }

                else{                                   //count is 1 , so we take the location only from the list

                    char cc[10];
                    char* temp  = node_location(node);
                    sprintf(cc,"%d",1);
                    write(filedes2,temp,strlen(temp));          //write the location to the .out file along with 1 
                    write(filedes2,"\t",strlen("\t"));
                    write(filedes2,cc,strlen(cc));
                    write(filedes2,"\n",strlen("\n"));
                    free(temp);                                 //note List_count does not free any memory , so we do it here
                    if(node != NULL){

                        free(node);

                    }

                }

                node = list_count_remove(list);             //remove next node from List_count and go back to loop

            }

            close(filedes2);

        }

        if(raise(SIGSTOP) != 0){                        //after worker has finished writing all locations

            perror("SIGSTOP failed");                   //we use raise(SIGSTOP) to change its state to stopped , then manager handles SIGCHLD signal

        }      
    }
    
    free(fifo_name);                                    //free dynamically allocated memory
    free(dynamic_buff);
    list_count_destroy(list);

}

void exit_handler(int signum){                          //exit_handler is used when SIGINNT signal is sent from manager
                                                        //basically it deletes all global,dynamicallly allocated memory and exits 

    if(signum == SIGINT){  
        
        if(fifo_name != NULL){
            
            unlink(fifo_name);
            free(fifo_name);

        }

        if(dynamic_buff != NULL){

            free(dynamic_buff);            

        }

        if(list != NULL){

            list_count_destroy(list);

        }


        exit(0);
    }

}