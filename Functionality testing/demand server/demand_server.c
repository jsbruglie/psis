#include "server_utils.h"

// Global hashtable
Hashtable* hashtable;

// Logging related variables
pthread_mutex_t log_lock;
FILE* log_fp;

// Socket global variables
int sock_fd;

// Perform a backup
int backup(){
	lockHashtable(hashtable);	
	writeBackupHashtable(hashtable, BACKUP_FILE);	
	pthread_mutex_lock(&log_lock);
	log_fp = eraseLog(log_fp, LOG_FILE);
	pthread_mutex_unlock(&log_lock);
	unlockHashtable(hashtable);
}

// Handle CTRL-C Signal
void intHandler(){
	
	// Close and unlink socket
	close(sock_fd);
	printf("\nSuccessfully closed socket.\n"); // DEBUG
	backup();	
	freeHashtable(hashtable);
	fclose(log_fp);
	exit(0);
}

void* clientHandler(void* pthread_arg){

	int client_fd = *((int*) pthread_arg);
	int ret = 0;

	// Message related variables
	message m;
	int nbytes;

	while ((nbytes = recv(client_fd, &m, sizeof(message), 0)) != 0){
		printf("Request from client %d: FLAG %d KEY %d SIZE %d\n", client_fd, m.flag, m.key, m.value_length); 
		if((ret = processRequest(client_fd, m, hashtable, log_fp, &log_lock)) != 0){
			printf("Data Sv - Request was not fulfilled. Return value %d.\n", ret); //DEBUG
		}
		printHashtable(hashtable);
	}
	return NULL;
}

int main(int argc, char **argv){

	// Thread related variables
	pthread_t new_thread_id;

	// Read backup
	hashtable = (Hashtable*) restoreFromFile(BACKUP_FILE, HASHTABLE_SIZE);
	printHashtable(hashtable);
	
	// Open log file and apply entries to the hashtable. Initialize log file mutex lock
	log_fp = (FILE*) processLogEntries(LOG_FILE, hashtable);
	pthread_mutex_init(&log_lock, NULL);	
	
	printHashtable(hashtable);
	
	// Configure server and client adress variables
	struct sockaddr_in server_addr, client_addr;
	socklen_t size_addr;
	int client_fd, new_client_fd;
	size_addr = sizeof(client_addr);	
	
	// Creates a socket for the server
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("Socket: ");
		exit(-1);
 	}
 	
 	// Communication protocol configuration
	server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DATA_PORT);

    // Server bind
 	if( bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		perror("Sv - Bind: ");
		exit(-1);
	}
 	printf("Socket created and binded.\nReady to receive messages...\n");
	
	listen(sock_fd, MAX_CLIENTS);	
	
	// Configure CTR-C signal
	signal(SIGINT, intHandler);

	while ( (client_fd = accept(sock_fd,(struct sockaddr *)&client_addr, &size_addr)) != 0){
		printf("New file descriptor is %d\n", client_fd);
		if(client_fd == -1){
			perror("Sv - Accept: ");
			exit(-1);
		}
		new_client_fd = client_fd;
		pthread_create(&new_thread_id, NULL, (void*) clientHandler, (void*) &new_client_fd);
		pthread_detach(new_thread_id);
	}
}
