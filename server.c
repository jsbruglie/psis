#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "LinkedList.h"


#define forever() while(1)

/* Handle CTRL-C Signal: call signal(SIGINT, intHandler); in main*/
void intHandler(int sock_fd){
	close(sock_fd);
	unlink(SOCK_ADDRESS);
	printf("\nSuccessfully closed and unlinked socket.\n");
	exit(0);
}

int main(int argc, char **argv){


	LinkedList *lp;
	LinkedList *aux;
	lp = NULL;



	message m;
	message temp_msg;

	int nbytes;

	/* Configure CTR-C signal */
	signal(SIGINT, intHandler);
	
	struct sockaddr_in server_addr, client_addr;
	socklen_t size_addr;	
	
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

 	printf("Socket created and binded.\nReady to receive messages...\n");
 	 	
	listen(sock_fd, 20);	
	forever(){
		
		size_addr = sizeof(client_addr);	
		int new_fd = accept(sock_fd,(struct sockaddr *)&client_addr, &size_addr);
		printf("New file descriptor is %d\n", new_fd);
		if(new_fd == -1){
			perror("Accept: ");
			exit(-1);
		}





		/* Receive a message */
       	nbytes = recv(new_fd, &m, sizeof(message), 0); 
		printf("Received %d bytes: %s with key %u \n", nbytes, m.value, m.key);
		
		if(!strcmp(m.value,"")){

			if(m.key == 30){
				printf("Fetching a value for key %u...\n", m.key);

				/*Iterate through the list*/
				aux=lp;
				while(aux != NULL){
					temp_msg = getMessageLinkedList(aux);
					if(temp_msg.key == m.key){
						break;
					}
					aux=getNextNodeLinkedList(aux);
				}


				strcpy(m.value, temp_msg.value);




				printf("Found value %s\n", m.value);
				m.value[VALUE_LEN - 1] = '\0'; 	
        		nbytes = send(new_fd, &m, sizeof(message), 0);
			}

		}else{
			/*Insert into the list*/
			lp = insertUnsortedLinkedList(lp, m);
			printf("succesful insert\n");
			//Write to the list. If the write is succesful acknowledge the client
			/*
			strcpy(m.value, "s");
			m.value[VALUE_LEN - 1] = '\0'; 	
    		nbytes = send(new_fd, &m, sizeof(message), 0);
			*/
		}





	}

	exit(0);
}
