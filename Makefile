CC = gcc 
CFLAGS = -std=c11 -Wpedantic -Wall -Wextra -Wconversion -Werror -D_XOPEN_SOURCE=500
LDFLAGS = -lrt -pthread
OBJ = $(wildcard *.o)
EXECUTABLE = daemon client

all: $(EXECUTABLE)

daemon: daemon.o sllist.o shared_memory.o priority_queue.o utils.o threads.o
	$(CC) -o $@ $^ $(LDFLAGS)
	
client: client.o sllist.o shared_memory.o priority_queue.o utils.o threads.o
	$(CC) -o $@ $^ $(LDFLAGS)

client.o: client.c defines.h sllist.h priority_queue.h threads.h utils.h \
 shared_memory.h
daemon.o: daemon.c defines.h sllist.h shared_memory.h priority_queue.h \
 utils.h threads.h
priority_queue.o: priority_queue.c defines.h sllist.h priority_queue.h
shared_memory.o: shared_memory.c defines.h sllist.h
sllist.o: sllist.c sllist.h
threads.o: threads.c defines.h sllist.h utils.h
utils.o: utils.c defines.h sllist.h

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $< 
	
clean:
	rm -rf $(OBJ) $(EXECUTABLE)
