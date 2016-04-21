#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>

#include "psiskv.h"

int kv_connect(char* kv_server_ip, int kv_server_port){
	
	struct sockaddr_in server_addr;	
	int err;	

    /* Open a socket */
 	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
 	if (sock_fd == -1){
		perror("kv_connect - Socket: ");
		exit(-1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(kv_server_ip); 	/* Set destination IP number - localhost, 127.0.0.1*/ 
	server_addr.sin_port = htons(kv_server_port);                		/* Set destination port number */
	
	/* Connect to the server */
	err = connect(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr));
	if (err == -1){
		perror("kv_connect - Connect: ");
		return -1;
	}
	return sock_fd;
}

/* 
	void kv_close
	Closes the connection with a previously opened key-value store.
*/
void kv_close(int kv_descriptor){
	printf("Closing socket...\n");
	close(kv_descriptor);
}

/* 
	int kv_write	
	Contacts the key-value store represented by kv_descriptor and stores the pair (key,value).
	Returns 0 on success, -1 on failure.
*/ 
int kv_write(int kv_descriptor, uint32_t key, char* value, int value_length){
	
	message m;
	int nbytes;
	strcpy(m.value, value);
	m.key = key;
	m.flag = WRITE;
	nbytes = send(kv_descriptor, &m, sizeof(message), 0);
	if (nbytes==-1){
		perror("kv_write - Error sending: ");
		return -1;
	}
	printf("Sent %d bytes: %s with key %u \n", nbytes, m.value, m.key);

	//The server should return something to acknowledge
	/*
	nbytes = recv(kv_descriptor, &m, sizeof(message), 0);
	if(strcmp(m.value,"s")){
		printf(" %d bytes were writen on the server. %s", nbytes, m.value);
		return 0;
	}else{
		perror("kv_write - Error on the server");
		return -1;
	}*/
	return 0;


}

/* 
	int kv_read
	Contacts the key-value store represented by kv_descriptor and retrieves the value corresponding to the given key.
	The value field accepts an array of bytes with the length in value_length, and is passed by reference.
	Returns 0 on success, -1 on failure. If a value has previously been deleted, and there is an attempt to read it,it should result on error.  
*/
int kv_read(int kv_descriptor, uint32_t key, char* value, int value_length){
	
	//Must write to make the server know and then read
	message m;
	int nbytes;

	strcpy(m.value,"READ");
	m.key = key;
	m.flag = READ;
	nbytes = send(kv_descriptor, &m, sizeof(message), 0);
	if (nbytes == -1){
		perror("kv_read - Send: ");
		return -1;
	}
	
	nbytes = recv(kv_descriptor, &m, sizeof(message), 0);
	if (nbytes == -1){
		perror("kv_read - Recv: ");
		return -1;
	}
	
	printf("Received %d bytes. %s \n", nbytes, m.value);
	strcpy(value, m.value);
	
	return 0;

}

/* 
	int kv_delete
	Contacts the key_value store to delete the value corresponding to a given key.
*/
int kv_delete(int kv_descriptor, uint32_t key){

}


