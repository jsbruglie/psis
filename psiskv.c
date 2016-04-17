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
		perror("Socket: ");
		exit(-1);
	}
	
	server_addr.sin_family = AF_INET;

	//This might not be correct
	server_addr.sin_addr.s_addr = inet_addr(kv_server_ip); 	/* Set destination IP number - localhost, 127.0.0.1*/ 
   	


   	server_addr.sin_port = htons(kv_server_port);                		/* Set destination port number */
	
	/* Connect to the server */
	err = connect(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr));
	if (err == -1){
		perror("Connect: ");
		exit(-1);
	}
	return sock_fd;

}
