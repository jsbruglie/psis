#include "storyserver.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(){
    
    message m;
	
	struct sockaddr_in server_addr;	
	
	int nbytes;
	int err;	

	char buffer[100];	

    /* Open a socket */
 	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
 	if (sock_fd == -1){
		perror("Socket: ");
		exit(-1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); 	/* Set destination IP number - localhost, 127.0.0.1*/ 
   	server_addr.sin_port = htons(PORT);                		/* Set destination port number */
	
	/* Connect to the server */
	err = connect(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr));
	if (err == -1){
		perror("Connect: ");
		exit(-1);
	}

	/* Scan for keyboard input */ 
    printf("Input: ");
    fgets(buffer, MESSAGE_LEN, stdin);
	strncpy(m.buffer, buffer, MESSAGE_LEN);
	m.buffer[MESSAGE_LEN - 1] = '\0'; 
	
    /* Send message to the server */
 	nbytes = send(sock_fd, &m, sizeof(message), 0);
	printf("Sent %d bytes: %s \n", nbytes, m.buffer);
	
	/* Get a response from the server */
	nbytes = recv(sock_fd, &m, sizeof(message), 0);
	printf("Received %d bytes. Story:  %s", nbytes, m.buffer);	
	
	close(sock_fd);
	exit(0);
}
