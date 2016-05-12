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
	int i, err = -1;	
	
    // Open a socket
 	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
 	if (sock_fd == -1){
		perror("kv_connect - Socket");
		return(ERROR);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(kv_server_ip); 		// Set destination IP number // 
	server_addr.sin_port = htons(kv_server_port);               // Set destination port number //
	
	// Connect to the server
	for (i = 0; (err == -1 ) && (i < NUMBER_OF_TRIES); i++){
		server_addr.sin_port = htons(kv_server_port + i);
		err = connect(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr));
	}
	if (err == -1){
		perror("kv_connect - Couldn't connect to the server");
		return ERROR;
	}
	
	//Send a message to the front server so the data port can be sent
	int DS_port;
	message m;
	memset(&m, 0, sizeof(message));
	m.flag = CLIENT_TAG;
	int nbytes = send(sock_fd, &m, sizeof(message), 0);

	//Wait to received a message to get the data server port
	nbytes = recv(sock_fd, &DS_port, sizeof(DS_port), 0);
	if (DS_port == -1){
		printf("\tkv_connect - Front server is unavailable.\n");
		return ERROR;
	}
	printf("\tkv_connect - Received the port of the Data Server %d\n", DS_port);
	close(sock_fd);

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
 	if (sock_fd == -1){
		perror("kv_connect - Socket");
		return ERROR;
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(kv_server_ip);
	server_addr.sin_port = htons(DS_port);               // Connect to the data server
	err = connect(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr));
	if (err == -1){
		perror("kv_connect - Connect");
		return ERROR;
	}
	return sock_fd;
}

/* 
	void kv_close
	Closes the connection with a previously opened key-value store.
*/
void kv_close(int kv_descriptor){
	close(kv_descriptor);
}

/* 
	int kv_write	
	Contacts the key-value store represented by kv_descriptor and stores the pair (key,value).
	Returns 0 on success, -1 (ERROR) on failure.
*/ 
int kv_write(int kv_descriptor, uint32_t key, char* value, int value_length, int kv_overwrite){
	
	int ret = 0;
	int nbytes;
	message m;
	m.key = key;
	m.value_length = value_length;	
	
	if (kv_overwrite == 0){  
		m.flag = WRITE;
	}else{
		m.flag = OVERWRITE;
	}

	// Send a message with the key and the size of the data inserted
	nbytes = send(kv_descriptor, &m, sizeof(message), 0);
	if (nbytes == -1){
		perror("kv_write - Error sending key and size: ");
		return ERROR;
	}
	printf("\tkv_write - Request to server %d: FLAG %d KEY %d SIZE %d\n", kv_descriptor, m.flag, m.key, m.value_length); // DEBUG
				
	// Send the actual data
	nbytes = send(kv_descriptor, value, value_length, 0);
	nbytes = recv(kv_descriptor, &m, sizeof(message), 0);
	if (nbytes == -1){
		perror("kv_write - Server unresponsive: "); //DEBUG
		ret = ERROR;
	}	
	else{
		ret = m.flag;
	}		
	return ret;
}

/* 
	int kv_read
	Contacts the key-value store represented by kv_descriptor and retrieves the value corresponding to the given key.
	The value field accepts an array of bytes with the length in value_length, and is passed by reference.
	Returns 0 on success, -1 on failure. If a value has previously been deleted, and there is an attempt to read it,it should result on error.  
*/
int kv_read(int kv_descriptor, uint32_t key, char* value, int value_length){

	int ret = 0;
	int nbytes;
	message m;
	memset(&m, 0, sizeof(message));	
	m.key = key;
	m.flag = READ;
	char buffer[MAX_BUFFER]; 

	nbytes = send(kv_descriptor, &m, sizeof(message), 0);
	printf("\tkv_read - Request to server %d: FLAG %d KEY %d SIZE %d\n", kv_descriptor, m.flag, m.key, m.value_length); // DEBUG

	nbytes = recv(kv_descriptor, &m, sizeof(message), 0);
	if (nbytes == -1){
		perror("kv_read - Failed to get answer from server"); // DEBUG
		ret = ERROR;
	}else if(m.flag == ERROR){
		printf("kv_read - Value not found"); // DEBUG
		ret = ERROR;
	}
	else{
		nbytes = recv(kv_descriptor, buffer, value_length, 0);
		memcpy(value, buffer, value_length); 
		printf("\tkv_read - Found %s.\n", value);
		ret = 0;
	}	
	return ret;
}

/* 
	int kv_delete
	Contacts the key_value store to delete the value corresponding to a given key.
*/
int kv_delete(int kv_descriptor, uint32_t key){

	int ret = 0;
	int nbytes;
	message m;
	m.key = key;	
	m.flag = DELETE;
	m.value_length = -1;

	nbytes = send(kv_descriptor, &m, sizeof(message), 0);
	printf("Request to server %d: FLAG %d KEY %d SIZE %d\n", kv_descriptor, m.flag, m.key, m.value_length); // DEBUG

	nbytes = recv(kv_descriptor, &m, sizeof(message), 0);
	if (nbytes == -1){
		perror("kv_delete - Failed to get answer from server.\n"); // DEBUG
		ret = ERROR;
	}else if(m.flag == ERROR){
		printf("kv_delete - Error.\n"); // DEBUG
		ret = ERROR;
	}			
	return ret;

}

