#include "server_utils.h"
#include "server_common.h"

#define MIN_THREADS 2
#define MAX_THREADS 5

// Global hashtable
Hashtable* hashtable;

// Logging related variables
pthread_mutex_t log_lock;
FILE* log_fp;

// Socket global variables
int data_sock_fd;
int front_sock_fd;

// Threadpool global variables
pthread_mutex_t pool_lock;
int ready_count = 0;
int creating_threads = 0;

// Function prototypes
void* clientHandler(void* pthread_arg);

// Creates the needed number of threads
void* createThreads(void* pthread_arg){

	pthread_t new_thread_id;
	int i;

	pthread_mutex_lock(&pool_lock);
	int thread_number = MAX_THREADS - ready_count;
	pthread_mutex_unlock(&pool_lock);

	for (i = 0; i < thread_number; i++){
		pthread_create(&new_thread_id, NULL, (void*) clientHandler, (void*)(NULL));
		pthread_detach(new_thread_id);
	}

	pthread_mutex_lock(&pool_lock);
	ready_count+= thread_number;
	creating_threads = 0;
	printf("[DS - cT]\tCreated a few threads! Ready count: %d\n", ready_count);
	pthread_mutex_unlock(&pool_lock);
}

// Perform a backup
int backup(){
	lockHashtable(hashtable);	
	writeBackupHashtable(hashtable, BACKUP_FILE);	
	pthread_mutex_lock(&log_lock);
	log_fp = eraseLog(log_fp, LOG_FILE);
	pthread_mutex_unlock(&log_lock);
	unlockHashtable(hashtable);
}

void quitDataServer(){

	close(data_sock_fd);
	close(front_sock_fd);
	printf("[DS - quit]\tSuccessfully closed socket.\n"); // DEBUG
	backup();	
	freeHashtable(hashtable);
	fclose(log_fp);
	exit(0);
}

void* clientHandler(void* pthread_arg){

	int ret = 0;

	// Thread management variables
	pthread_t thread_creator_id;

	// Client adress related variables
	int client_fd;
	struct sockaddr_in client_addr;	
	int size_addr = sizeof(client_addr);	

	// Message related variables
	message m;
	int nbytes;	

	client_fd = accept(data_sock_fd,(struct sockaddr *)&client_addr, &size_addr);
	if(client_fd == -1){
		perror("Data Sv - Accept");
		exit(-1);
	}
	// If we have reached a critically low number of threads, create a new thread which single 
	//	purpose is to create the necessary number of client handling threads.
	pthread_mutex_lock(&pool_lock);
	ready_count--;
	printf("[DS - cH]\tReady thread count %d\n", ready_count);
	if (!creating_threads){
		if (ready_count <= MIN_THREADS){
			creating_threads = 1;
			pthread_create(&thread_creator_id, NULL, (void*) createThreads, (void*)(NULL));
			pthread_detach(thread_creator_id);
		}
	}
	pthread_mutex_unlock(&pool_lock);
	
	while ((nbytes = recv(client_fd, &m, sizeof(message), 0)) != 0){
		printf("[DS - cH]\tRequest from client %d\tFLAG %d KEY %d SIZE %d\n", client_fd, m.flag, m.key, m.value_length); 
		if((ret = processRequest(client_fd, m, hashtable, log_fp, &log_lock)) != 0){
			printf("[DS -cH]\tData Sv - Request from client %d\twas not fulfilled. Return value %d.\n", client_fd, ret); //DEBUG
		}
		printHashtable(hashtable);
	}
	pthread_exit(NULL);
}

int setupFrontServer(){

	struct sockaddr_in server_addr;	
	int err = -1;	

 	front_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
 	if (front_sock_fd== -1){
		perror("[DS - fSH]\tSocket");
		exit(-1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  

	int i;

	// Connect to the server
	for(i = 0; err == -1 && i < NUMBER_OF_TRIES; i++){
		server_addr.sin_port = htons(FS_PORT + i);
		err = connect(front_sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr));
	}
	
	if (err == -1){
		perror("[DS - fSH]\tConnect");
		exit(-1);
	}
	
	printf("[DS - fSH]\tExchanging messages with the front server at port %d.\n", (FS_PORT + i - 1));
	
	int nbytes;	
	message m;
	memset(&m, 0, sizeof(message));
	
	int port = DS_PORT;
	m.flag = DS_SERVER_TAG;
	printf("[DS - fSH]\tSending the message with the data server tag.\n");
	nbytes = send(front_sock_fd, &m, sizeof(message), 0);
	printf("[DSH - fSH]\tSending the data server port.\n");
	nbytes = send(front_sock_fd, &port, sizeof(port), 0);

	return 0;
}

// Checks if the front server is alive
void frontServerHandler(){

	int hbeat = 1;

	while(1){
		hbeat = heartbeat(front_sock_fd, FIRST);
		if(hbeat != 0){
			printf("[DS - fSH]\tThe front server has gone down. Exiting.\n");
			quitDataServer();
		}
	}
}

int main(int argc, char **argv){

	// Read backup
	hashtable = (Hashtable*) restoreFromFile(BACKUP_FILE, HASHTABLE_SIZE);
	
	// Open log file and apply entries to the hashtable. Initialize log file mutex lock
	log_fp = (FILE*) processLogEntries(LOG_FILE, hashtable);
	pthread_mutex_init(&log_lock, NULL);

	printHashtable(hashtable);

	// Creates a socket for the server
	if ((data_sock_fd = createSocket(DS_PORT)) == -1)
		quitDataServer();
 	printf("[DS - main]\tSocket created and binded. Data server ready to receive messages.\n"); // DEBUG

 	// Configure CTR-C signal
	signal(SIGINT, quitDataServer);

 	// Configures the connection with the front server
	setupFrontServer();

	listen(data_sock_fd, 5);

	// Create the initial MAX_THREADS threads
	createThreads(NULL);

	// Keeps a connection with the front server and the main thread from exiting
	frontServerHandler();
}
