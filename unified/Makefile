DATA_SVR_OBJECT_FILES = data_server.o server_common.o psiskv.o server_utils.o LinkedList.o Hashtable.o
FRNT_SVR_OBJECT_FILES = front_server.o server_common.o psiskv.o
CLT_OBJECT_FILES = client.o psiskv.o

CFLAGS = -ggdb -Wall -pthread
LIBS = -lpthread 
CC = gcc  

all: data_server front_server client cleanup
data_server: $(DATA_SVR_OBJECT_FILES)  
front_server: $(FRNT_SVR_OBJECT_FILES)
client: $(CLT_OBJECT_FILES) 

client data_server front_server:
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@  

client.o data_server.o front_server.o: 

%.o: %.c
	$(CC) -c $<  

cleanup:
	rm -f *.o

clean:
	rm -f client data_server front_server Hashtable LinkedList psiskv *.o *~ 
