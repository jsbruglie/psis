#include <pthread.h>

#include "server_utils.h"

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

	int new_fd = *((int*) thread_arg);

	message m;
	int nbytes;

	/* Tend to a single client */
	while ((nbytes = recv(new_fd, &m, sizeof(message), 0)) != 0){
 
		printf("Received %d bytes: %s with key %u and flag %d \n", nbytes, m.value, m.key, m.flag);
		if(processRequest(new_fd, m, &lp)){
			printf("Error reading or writing\n");
		} //Send new_fd, message and the list pointer
		if (lp == NULL){
			printf("Something went wrong and the list is now NULL!\n");
		}
	}
	//pthread_exit((void*) thread_arg);
}

int main(int argc, char **argv){

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

 	int client_fd;
	listen(sock_fd, 20);	
	size_addr = sizeof(client_addr);	
		
	while ( (client_fd = accept(sock_fd,(struct sockaddr *)&client_addr, &size_addr)) != 0){
		printf("New file descriptor is %d. Thread id is %d\n", client_fd, i);
		if(client_fd == -1){
			perror("Sv - Accept: ");
			exit(-1);
		}
		
		pthread_attr_t attr;

		/* Set the thread as detached, so that it will free its resources upon task completion - avoid memory leaks */
		int rc;
		rc = pthread_attr_init(&attr);
		rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		
		pthread_create(&(thread_id[i++]), &attr, (void*) pthreadHandler, (void*) &client_fd);
	}
	//pthread_join(thread_id[0],NULL);
}
