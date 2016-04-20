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
#include "psiskv.h"

#define forever() while(1)

#define MAX_CLIENTS 10

/* Global data structure for kv store */
LinkedList* lp;

/* Handle CTRL-C Signal: call signal(SIGINT, intHandler); in main*/
void intHandler(int sock_fd){
	
	/* Close and unlink socket */
	close(sock_fd);
	unlink(SOCK_ADDRESS);
	printf("\nSuccessfully closed and unlinked socket.\n");
	/* Write kv store to file and delete memory? just free memory ?*/
	freeLinkedList(lp, kv_freeKvPair);
	printf("Successfully freed the list\n");
	pthread_exit(NULL);
}

void* pthreadHandler(void* thread_arg){

	int* t_aux = (int*) thread_arg;
	int new_fd = t_aux[0];
	int thread_id = t_aux[1]; 

	// printf(">>Thread number %ld\n", pthread_self());
	
	LinkedList* aux;

	message m;
	kv_pair* temp_kv = NULL;
	kv_pair* new_kv = NULL;

	int nbytes;

	/* Tend to a single client */
	while ((nbytes = recv(new_fd, &m, sizeof(message), 0)) != 0){
   		 
		printf("Received %d bytes: %s with key %u \n", nbytes, m.value, m.key);
	
		if(!strcmp(m.value,"READ")) {
			
			printf("Read access requested for key %d\n", m.key);
			if(m.key == 30){
				
				printf("Fetching a value for key %u...\n", m.key);
				/* Iterate through the list - This should really have its own function tbh */
				for(aux = lp; aux != NULL; aux = getNextNodeLinkedList(aux)){
					temp_kv = (kv_pair*) getItemLinkedList(aux);
					if(temp_kv->key == m.key){
						break;
					}
				}	

				strcpy(m.value, temp_kv->value);
				printf("Found value %s\n", m.value);
				m.value[MAX_LEN - 1] = '\0'; 	
	    		nbytes = send(new_fd, &m, sizeof(message), 0); /* This is pretty messy */
			}
		}else{
			
			/* Insert into the list*/
			new_kv = kv_allocKvPair(m.key, m.value, strlen(m.value));
			lp = insertUnsortedLinkedList(lp, new_kv);
			
			/* Test that the item has indeed been inserted */
			new_kv = (kv_pair*) getItemLinkedList(lp);
			printf("Succesful insertion of kv pair: %d %s\n", new_kv->key, new_kv->value);
		}
	}
	pthread_exit((void*) thread_arg);
}

int main(int argc, char **argv){

	LinkedList* aux;

	message m;
	kv_pair* temp_kv = NULL;
	kv_pair* new_kv = NULL;

	int nbytes;

	/* Configure CTR-C signal */
	signal(SIGINT, intHandler);
	
	/* Configure server and client adress variables */
	struct sockaddr_in server_addr, client_addr;
	socklen_t size_addr;	
	
	/* Creates a socket for the server */
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("Socket: ");
		exit(-1);
 	}
 	
 	/* Communication protocol configuration */
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    /* Server bind */
 	int err = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
 	if(err == -1) {
		perror("Sv - Bind: ");
		exit(-1);
	}
 	printf("Socket created and binded.\nReady to receive messages...\n");
 	
 	/* Create pool of threads */

 	pthread_t thread_id[MAX_CLIENTS];
 	int i = 0;

	listen(sock_fd, 20);	
	forever(){

		/* Perform accept - should be moved to a new handler function */
		size_addr = sizeof(client_addr);	
		int new_fd = accept(sock_fd,(struct sockaddr *)&client_addr, &size_addr);
		printf("New file descriptor is %d. Thread id is %d\n", new_fd, i);
		if(new_fd == -1){
			perror("Sv - Accept: ");
			exit(-1);
		}
		int thread_arg[2];
		thread_arg[0] = new_fd;
		thread_arg[1] = i;

		pthread_create(&(thread_id[i]), NULL, (void*) pthreadHandler, thread_arg);
		pthread_join(thread_id[i++]);
	}
}
