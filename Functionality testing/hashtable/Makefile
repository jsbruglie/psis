OBJECT_FILES = main.o LinkedList.o Hashtable.o

CFLAGS = -ggdb -Wall -pthread
LIBS = -lpthread 
CC = gcc  

all: main cleanup
main: $(OBJECT_FILES) 

main:
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@  

main.o: 

%.o: %.c
	$(CC) -c $<  

cleanup:
	rm -f *.o

clean:
	rm -f client server psiskv *.o *~ 