#include "storyserver.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define forever() while(1)

/* Handle CTRL-C Signal*/
void intHandler(int sock_fd){
	close(sock_fd);
	unlink(SOCK_ADDRESS);
	printf("\nSuccessfully closed and unlinked socket.\n");
	exit(0);
}

int main(){
    
	message m;
 	
	int i = 0;

	/* Configure CTR-C signal */
	signal(SIGINT, intHandler);

	/* Setup a string that will be the complete story */
	char* story = strdup("");
	
	struct sockaddr_in server_addr, client_addr;
	socklen_t size_addr;	
	
	int nbytes;
	
	/* Creates a socket for the server */
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("Socket: ");
		exit(-1);
 	}
 
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

 	int err = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
 	if(err == -1) {
		perror("Bind: ");
		exit(-1);
	}

 	printf("Socket created and binded.\nReady to receive messages\n");
 	 	
	listen(sock_fd, 20);	
	forever(){
        
		size_addr = sizeof(client_addr);	
		int new_fd = accept(sock_fd,(struct sockaddr *)&client_addr, &size_addr);
		if(new_fd == -1){
			perror("Accept: ");
			exit(-1);
		}

		/* Receive a message */
       	nbytes = recv(new_fd, &m, sizeof(message), 0); 
		printf("Received %d bytes: %s", nbytes, m.buffer);	
        
		/* Process message */
		strcat(story, m.buffer);
		printf("Current story:\n%s", story);
		
		/* Send a message back */
		strncpy(m.buffer, story, MESSAGE_LEN);
		m.buffer[MESSAGE_LEN - 1] = '\0'; 	
        nbytes = send(new_fd, &m, sizeof(message), 0);
		
		close(new_fd);	
    }
    exit(0);
}
