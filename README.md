# Multiple-Process-Synchronization-and-Communication-for-Managing-Files-
System for detecting new/modified files on given directory (through inotifywait) . Reading files and detecting url links , then saving location of link on .out txt file.



## Assignment 1 (85%)

### Structs that i have used (Queue.c , List_count.c)

- About the implementation of the project i have created a `Queue` (Queue.c/Queue.h) , which uses a List , in order to save all workers that are in a stopped state and have finished writing in      .out file.

- I have created a `List` (List_count.c) , which is used by the workers. Its function is to save all locations that the workers have found in the file they are reading . if the location does not 
  exist in the List we add it and count is set to 1 , if it already exists we increment the count by 1. Then the worker removes the elements of the List 1 by 1 and writes them in .out file. 

### thisdir , results directories

- If manager is run with no arguments (./sniffer) , then inotifywait monitors thisdir directory.

- Results directory is used by the worker to create all .out files.

### manager.c

- File `manager.c` has the main function of the whole project. First of all , i define all the variables that the manager will use  along with 2 Queues . worker_queue saves all stopped workers      and workerpid_queue saves all pids from workers that have been created.

- `Manager` creates a pipe from which it will read all events that inotifywait will return and then forks. The child proccess is the listener and executes the execv command for inotifywait ,        `strtok` splits the buff and takes the path of the file. 

- Then , `manager` checks if worker_queue is empty and if it is creates a new worker with `fork()`. Father process of `fork()` is responsible for creating a named `pipe` with path                   `/tmp/workerpid` , where    pid is the process id of the child and then writes the path which was returned from `inotifywait`. If `worker_queue` was not empty we delete 1st worker pid from the     queue , send `SIGCONT` to continue   the stopped worker and write in previously created named `pipe` the path that `inotifywait` returned.

- Finally , we have defined 2 sigaction structs and 2 handlers . `Act1` and `child_handler` is used , in order to catch the `SIGCHLD` with sigaction(SIGCHLD,&act1,NULL) , which exists because of    raise(SIGSTOP) inside the worker . `child_handler` then with while((pid = waitpid(-1,&status,WNOHANG | WUNTRACED)) > 0) takes all worker pids that are in stopped state and inserts them to         worker_queue.

- `Act2` and `exit_handler` is used to catch `SIGINT` signal (ctrl^c) from user to terminate. `exit_handler` deletes all pids from `workerpid_queue`  and sends `SIGCONT` signal. State of all         workers changes    from stopped to continued and `SIGINT` is send automatically to all workers , otherwise if their state was stopped `SIGINT` would be ignored. We free all dynamically            allocated memory and exit the program with 0.


### worker.c

- `Worker` is a child proccess of `manager.c` and is executed with exec . First of all, worker creates the path of named pipe to communicate with manager , opens the `fifo` and reads the path to     file returned from `inotifywait` inside a while loop. Then splits the file name and creates the .out file where all locations will written , .out file is saved in results directory. `Worker`      reads byte by byte the entire file when 7 or 11 bytes are read , worker checks if this is a url (compares dynamic_buff to http:// and http://www.) when a " " , or "\n" or "/" is read we split     the string read to find the location of the url depending on whether it is a url with www. or not.

- After location is found , it is saved in struct List_count or if we have already found this location counter in List_count is increased. After entire file is read worker deletes all nodes         inside List_count and writes the location ,"\t",counter and the "\n" to .out file.

- Worker uses function raise(SIGSTOP) to change its state to stopped to wait for next file from fifo , after writing all locations found to .out. We have defined a struct act2 and exit_handler .    When `SIGINT` is sent from user , `manager` receives it and it is automatically sent to all child processes , then we catch `SIGINT` inside `worker` with sigaction(SIGINT,&act2,NULL) where we     `free` all global dynamically allocated memory (dynamic_buf,List_count,fifo_name) and exit(0)


### listener  

- `Listener` does not have a dedicated .c file , because there would only be one function inside (exec) .

- Instead i chose to `fork` inside `manager.c` , then use dup2(fd[WRITE],1) , to redirect the output of inotifywait inside the pipe.

- If the program is executed without any arguments (./sniffer) , i call execv for inotifywait and the monitored directory is thisdir , else if there arguments i take the path from [-p path] , use   execv again but now the monitored directory is the directory inside the path. 

### Makefile :

- I have also created a `Makefile` for easier compilation of the project.

    - `make sniffer` creates a sniffer executable which is run with ./sniffer [-p path] (-p path is optional).
    - `make clean` erases sniffer and worker executables along with all .out files previously created.


## Assignment 2 (15%)


### finder.sh

- We run bash script finder.sh with ./finder.sh [TLD] [TLD] , we must howerver include at least one TLD to search for. `finder.sh` loops for all TLDs given as arguments and searches all .out        files inside results directory , then reads file line by line takes the location of the line and reverses it (TLD is already reversed) if reversed TLD matches with the first characters read       from reverse location(till length of TLD) then TLD was found and we increase num_of_appearances by 1 then we print the TLD and the num_of_appearances.

