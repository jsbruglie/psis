
/*
	psis_kv.h
	Header for the library that allows the connection between server and clients
*/

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Defines */
#define SOCK_ADDRESS "/tmp/psis_kv_sock"
#define PORT 9999

#define MAX_LEN 100

#define WRITE 1
#define READ 0
#define DELETE 2

/* 	Data structures */
typedef struct _message{
	uint32_t key;
    char value[MAX_LEN];
    int flag;
} message;


/*
	int kv_connect
	Establishes connection with key-value store.
	Returns -1 on failure and a positive int representing the contacted key-value store.
*/
int kv_connect(char* kv_server_ip, int kv_server_port);

/* 
	void kv_close
	Closes the connection with a previously opened key-value store.
*/
void kv_close(int kv_descriptor);

/* 
	int kv_write	
	Contacts the key-value store represented by kv_descriptor and stores the pair (key,value).
	Returns 0 on success, -1 on failure.
*/ 
int kv_write(int kv_descriptor, uint32_t key, char* value, int value_length);

/* 
	int kv_read
	Contacts the key-value store represented by kv_descriptor and retrieves the value corresponding to the given key.
	The value field accepts an array of bytes with the length in value_length, and is passed by reference.
	Returns 0 on success, -1 on failure. If a value has previously been deleted, and there is an attempt to read it,it should result on error.  
*/
int kv_read(int kv_descriptor, uint32_t key, char* value, int value_length);

/* 
	int kv_delete
	Contacts the key_value store to delete the value corresponding to a given key.
*/
int kv_delete(int kv_descriptor, uint32_t key);
