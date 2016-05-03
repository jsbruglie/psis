SVR_OBJECT_FILES = simple_server.o psiskv.o server_utils.o LinkedList.o Hashtable.o
CLT_OBJECT_FILES = client.o psiskv.o 

CFLAGS = -ggdb -Wall -pthread
LIBS = -lpthread 
CC = gcc  

all: client server cleanup
client: $(CLT_OBJECT_FILES) 
server: $(SVR_OBJECT_FILES)  

client server:
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@  

client.o server.o: 

%.o: %.c
	$(CC) -c $<  

cleanup:
	rm -f *.o

clean:
	rm -f client server psiskv *.o *~ 
