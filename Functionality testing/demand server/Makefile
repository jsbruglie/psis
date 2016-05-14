SIMPLE_SVR_OBJECT_FILES = simple_server.o psiskv.o server_utils.o LinkedList.o Hashtable.o
DEMAND_SVR_OBJECT_FILES = demand_server.o psiskv.o server_utils.o LinkedList.o Hashtable.o
CLT_OBJECT_FILES = client.o psiskv.o 

CFLAGS = -ggdb -Wall -pthread
LIBS = -lpthread 
CC = gcc  

all: client simple_server demand_server cleanup
client: $(CLT_OBJECT_FILES) 
simple_server: $(SIMPLE_SVR_OBJECT_FILES)  
demand_server: $(DEMAND_SVR_OBJECT_FILES)

client simple_server demand_server:
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@  

client.o simple_server.o demand_server: 

%.o: %.c
	$(CC) -c $<  

cleanup:
	rm -f *.o

clean:
	rm -f client simple_server demand_server psiskv *.o *~ 
