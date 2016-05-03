#include "server_utils.h"

// Global hashtable
Hashtable* hashtable;

// Threadpool global variables
pthread_mutex_t pool_lock;
int ready_count;

// Socket global variables
int sock_fd;

// Handle CTRL-C Signal: call signal(SIGINT, intHandler); in main
void intHandler(int sock_fd){
	
	// Close and unlink socket
	close(sock_fd);
	printf("\nSuccessfully closed socket.\n"); //DEUG
	freeHashtable(hashtable);
	// TODO - Perform proper cleanup
	// TODO - Backup
	exit(0);
}

void* clientHandler(void* pthread_arg){

	// Socket related variables
	struct sockaddr_in client_addr;
	int size_addr = sizeof(client_addr);	
	int client_fd;
	int ret;

	// Message related variables
	message m;
	int nbytes;

	// Tend to a single client

	client_fd = accept(sock_fd,(struct sockaddr *)&client_addr, &size_addr);
	
	if(client_fd == -1){
		perror("Data Sv - Accept: ");
		exit(-1);
	}
	
	while ((nbytes = recv(client_fd, &m, sizeof(message), 0)) != 0){
		printf("Request from client %d: FLAG %d KEY %d SIZE %d\n", client_fd, m.flag, m.key, m.value_length); 
		if((ret = processRequest(client_fd, m, hashtable)) != 0){
			printf("Data Sv - Request was not fulfilled. Return value %d.\n", ret); //DEBUG
		}
		printHashtable(hashtable);
	}
	return NULL;
}

int main(int argc, char **argv){
	
	// Create the hashtable
	hashtable = createHashtable(HASHTABLE_SIZE);

	// Configure CTR-C signal
	signal(SIGINT, intHandler);
	
	// Configure server and client adress variables
	struct sockaddr_in server_addr, client_addr;
	socklen_t size_addr;	
	
	// Creates a socket for the server
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("Data Sv - Socket: ");
		// TODO - Cleanup 
		exit(-1);
 	}
 	
 	// Communication protocol configuration
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DATA_PORT);

    // Server bind
 	int err = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
 	if(err == -1) {
		perror("Data Sv - Bind: ");
		// TODO - Cleanup
		exit(-1);
	}
 	printf("Socket created and binded. Data server ready to receive messages...\n"); // DEBUG
 	
	listen(sock_fd, MAX_CLIENTS);

	while(1){
		// TODO - While the front server has not ordered a shutdown	
		clientHandler(NULL);
	}
}
