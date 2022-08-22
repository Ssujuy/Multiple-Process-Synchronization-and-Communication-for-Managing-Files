CC = gcc
EXEC = sniffer
DIR = results
WOBJ = worker.o
MOBJ = manager.o
RMV = rm -f
Q = mods/Queue.c
LC = mods/List_count.c
M = manager.c
W = worker.c
WEXEC = worker

$(WOBJ) :
	$(CC) -c $(W)

$(MOBJ) :
	$(CC) -c $(M)

$(EXEC): $(WOBJ) $(MOBJ) 

	$(CC) -o $(WEXEC) $(WOBJ) $(LC)
	$(CC) -o $(EXEC) $(MOBJ) $(Q)

run : 

	./$(EXEC)

clean:
	$(RMV) $(MOBJ)
	$(RMV) $(WOBJ)
	$(RMV) $(EXEC)
	$(RMV) $(WEXEC)
	$(RMV) $(DIR)/*