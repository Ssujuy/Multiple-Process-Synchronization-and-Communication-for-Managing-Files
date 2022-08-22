#include "libs/Queue.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <signal.h>
#include <sys/wait.h> 
#include <sys/types.h>

extern int errno;
int flag = 0;                                       
Queue* worker_queue;                                        //global variables for exit handler to free , after SIGINT
Queue* workerpid_queue;
char* dynamic_buff;

void child_handler(int signum);
void exit_handler(int signum);

#define BUFFSIZE 100
#define READ 0
#define WRITE 1
#define PERMS 0666
#define FIFO "/tmp/worker"                                  //path to named pipes


int main(int argc , char* argv[]){

    pid_t pid,wpid;
    int writefd;
    char buff[BUFFSIZE];
    dynamic_buff = malloc(sizeof(buff));
    int buff_size = 1;
    int fd[2];
    int i;
    int size;
    int error;

    worker_queue = queue_create();                          //create worker queue and worker pid queue
    workerpid_queue = queue_create();
    
    static struct sigaction act1;                           //define struct sigaction act1 to handle SIGCHLD signal from workers
    act1.sa_handler = child_handler;
    sigemptyset(&(act1.sa_mask));

    static struct sigaction act2;                           //define struct sigaction act2 to handle SIGINT signal from terminal (Ctrl^c)
    act2.sa_handler = exit_handler;
    sigemptyset(&(act2.sa_mask));
    sigaction(SIGINT,&act2,NULL);                           //sigaction catcher SIGINT signal

    memset(buff,0,sizeof(buff));                            //make buff array empty

    if(pipe(fd) == -1){                                     //fd is now a pipe

        perror("Pipe");
        exit(1);

    }

    if((pid = fork()) == -1){                               //call fork to create listener

        perror("fork");
        exit(1);

    }

    if(pid > 0){                                            //this is the parent process , also known as manager

        while(1){                                           

            if((size = read(fd[READ],buff,BUFFSIZE)) > 0){                  //read event returned from inotifywait

                if(size == BUFFSIZE){                                       //if size of bytes read was 100(BUFFSIZE) 

                    strcat(dynamic_buff,buff);                              //use strcat to put the event in dynamic_buf , realloc and read again
                    buff_size++;
                    dynamic_buff = realloc(dynamic_buff,buff_size * BUFFSIZE * sizeof(char));
                    memset(buff,0,sizeof(buff));
                    continue;
                    
                }

                else{
                    
                    char* str;
                    strcat(dynamic_buff,buff);

                    char* token = strtok(dynamic_buff, " ");
                    i = 1;
                    char* temp = token;
                    
                    while (token != NULL) {                         //use strtok function to split the event read 

                        i++;
                        token = strtok(NULL, " ");

                        if(i == 3){

                            str = malloc((strlen(token) + strlen(temp) + 1) * sizeof(char));            //save path and file that caused the event
                            memset(str,0,sizeof(str));
                            strcat(str,temp);
                            strcat(str,token);

                        }
                    }

                    memset(buff,0,sizeof(buff));
                    memset(dynamic_buff,0,sizeof(buff));                    //empty both buffers

                    dynamic_buff = realloc(dynamic_buff,sizeof(buff));
                    buff_size = 1;

                    if(queue_empty(worker_queue) == 1){                     //checks if queue of workers is empty (no workers are available)

                        flag = 0;
                        
                        if((wpid = fork()) == -1){                          //fork to create a new worker

                            perror("fork");
                            exit(1);

                        }

                        if(wpid > 0){                                       //parent process of fork 

                            int* wlpid = malloc(sizeof(*wlpid));
                            *wlpid = wpid;
                            queue_insert(workerpid_queue,wlpid);
                            char nmb[20];
                            sprintf(nmb,"%d",wpid);                         //here we create the fifo path and fifo name , at the end i use the process id of the worker so that every fifo is unique
                            char* fifo_name = malloc((strlen(FIFO) + strlen(nmb) + 1) * sizeof(char));
                            strcpy(fifo_name,FIFO);
                            strcat(fifo_name,nmb);

                            if(((error = mkfifo(fifo_name,PERMS)) < 0) && (errno != EEXIST)){           //create named pipe , for manager-worker communication

                                perror("Can't create fifo");
                                exit(1);

                            }

                            if((writefd = open(fifo_name,O_WRONLY)) < 0){                           //manager opens named pipe with write only flag 

                                perror("Can't open FIFO");
                                exit(1);

                            }

                            if (write(writefd,str,strlen(str)) != strlen(str)){                     //then writes the file , which caused the event

                                perror("FIFO write error");
                                exit(1);

                            }

                            free(fifo_name);                        //free dynamically allocated memory
                            free(str);
                            sigaction(SIGCHLD,&act1,NULL);          //catch SIGCHLD from a worker that finished and called raise(SIGSTOP)

                        }

                        else{

                            char* args[] = {"./worker",NULL};               //this is the child process of previous fork() , we create a new worker and call exec for worker executable
                            execvp(args[0],args);

                        }

                    }

                    else{                                                   //queue was not empty , so there is a worker available 

                        int* wid = queue_delete(worker_queue);              //delete first worker in queue and create the named_pipe path 
                        char nmb[20];
                        sprintf(nmb,"%d",*wid);
                        char* fifo_name = malloc((strlen(FIFO) + strlen(nmb) + 1) * sizeof(char));                  
                        strcpy(fifo_name,FIFO);
                        strcat(fifo_name,nmb);

                        kill((pid_t)*wid,SIGCONT);                          //send signal SIGCONT to stopped worker
                        free(wid);

                        if((writefd = open(fifo_name,O_WRONLY)) < 0){       //open fifo and write the path to file that cause the event from inotifywait

                            perror("Can't open FIFO");
                            exit(1);

                        }

                        if (write(writefd,str,strlen(str)) != strlen(str)){

                            perror("FIFO write error");
                            exit(1);

                        }

                    }
                }
            }

            else if(size == -1){                        //this if checks if signals have cause read to return -1 

                if(errno == EINTR){                     //because program is not ready to terminate qwe continue reading

                    continue;

                }

                else{

                    break;

                }

            }
        }
    }

    else{                                       //this is the child process , also known as listener

        if(argc == 1){                          //checking if we have received a path (file to monitor with inotifywait) from terminal

            dup2(fd[WRITE],1);                                                                                          //redirect stdout from inotifywait to pipe
            char *args[]={"/usr/bin/inotifywait", "-m", "-e", "create", "-e", "moved_to","thisdir", NULL};              //by default we monitor thisdir
            execv(args[0],args);

        }

        else if(argc == 3){

            dup2(fd[WRITE],1);                                                                                          //redirect stdout from inotifywait to pipe
            char *args[]={"/usr/bin/inotifywait", "-m", "-e", "create", "-e", "moved_to",argv[2], NULL};                //take path that was given and monitor that directory with inotifywait
            execv(args[0],args);

        }

        else{

            printf("Error wrong arguments received!!\n");
            exit(1);

        }

    }
    
    free(dynamic_buff);

}

void child_handler(int signum){                     //this the child_handler and it is called when SIGCHLD is caught

    if(signum == SIGCHLD){                          //checks if signal is SIGCHLD

        int pid;
        int status;

        while((pid = waitpid(-1,&status,WNOHANG | WUNTRACED)) > 0){             //use waitpid(-1,&status,WNOHANG | WUNTRACED) to find all workers that have changed state and are stopped

            int* cpid = malloc(sizeof(*cpid));                                  //take their process id and push it to queue
            *cpid = pid;
            queue_insert(worker_queue,cpid);

        }
    }
}

void exit_handler(int signum){                         //this is the exit_handler and it is called when SIGINT is caught(ctrl^c)


    if(signum == SIGINT){                               //checks if signal is SIGINT 

        int pid;
        int status;
        char fname[100];
        while(1){                                       

            if(queue_empty(workerpid_queue) == 1){          //checks if queue of stopped workers is empty , if yes break loop

                break;

            }

            else{                                           //take pid from queue and send SIGCONT to worker

                int* wid = queue_delete(workerpid_queue);
                kill(*wid,SIGCONT);                         //because SIGINT is send automatically to all child processes , we only need to send SIGCONT signal
                free(wid);                                  //otherwise SIGINT signal will be ignored

            }


        }

        if(dynamic_buff != NULL){                           //free all dynamically allocated memory and exit program

            free(dynamic_buff);

        }

        if(worker_queue != NULL){

            while(1){

                if(queue_empty(worker_queue) == 1){

                    break;

                }

                else{

                    int* wid = queue_delete(worker_queue);
                    free(wid);

                }

            }
            
            queue_free(worker_queue);

        }

        if(workerpid_queue != NULL){

            queue_free(workerpid_queue);

        }

        exit(0);
      
    }
}

