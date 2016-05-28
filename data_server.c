#include "server_utils.h"
#include "server_common.h"

// Verbose
//#define VERBOSE 1 // Uncomment this line for verbose terminal output
#include "debug.h"

#define MIN_THREADS 3
#define MAX_THREADS 10

// Global hashtable
Hashtable* hashtable;

// Logging related variables
pthread_mutex_t log_lock;
FILE* log_fp;

// Socket global variables
int data_sock_fd = -1;
int front_sock_fd = -1;

int DS_port = -1;
int FS_connected = 0;

// Threadpool global variables
pthread_mutex_t pool_lock;
int ready_count = 0;
int creating_threads = 0;

// Function prototypes
void setupFrontServer(int port, int range);
void frontServerHandler();
void quitDataServer();

void* clientHandler(void* pthread_arg);

void checkThreads();
void* createThreads(void* pthread_arg);

int backup();
void quitDataServer();

// Main program
int main(int argc, char **argv){

	// Read backup
	hashtable = (Hashtable*) restoreFromFile(BACKUP_FILE, HASHTABLE_SIZE);

	// Open log file and apply entries to the hashtable. Initialize log file mutex lock
	log_fp = (FILE*) processLogEntries(LOG_FILE, hashtable);
	pthread_mutex_init(&log_lock, NULL);

	//printHashtable(hashtable);

	// Create a socket
	int i;
	for(i = 0; data_sock_fd == -1 && i < NUMBER_OF_TRIES; i++)
		data_sock_fd = createSocket(DEFAULT_DS_PORT + i);
	if (data_sock_fd== -1){
		exit(-1);
	}
	DS_port = DEFAULT_DS_PORT + i - 1;
 	debugPrint("[DS - main]\tSocket created and binded. Data server ready to receive messages.\n"); // DEBUG

 	// Configures the connection with the front server
	setupFrontServer(DEFAULT_FS_PORT, NUMBER_OF_TRIES);

	listen(data_sock_fd, 5);

	// Create the initial MAX_THREADS threads
	createThreads(NULL);

	// Periodically monitor the aliveness of the Front Server
	frontServerHandler();
}

void setupFrontServer(int port, int range){

	struct sockaddr_in server_addr;	
	int err = -1;	

 	front_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
 	if (front_sock_fd == -1){
		perror("[DS - sFS]\tSocket");
		exit(-1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(LOCALHOST);  

	int i;
	for(i = 0; err == -1 && i < range; i++){
		if (port + i != DS_port){ 
			server_addr.sin_port = htons(port + i);
			err = connect(front_sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr));
		}
	}
	if (err == -1){
		perror("[DS - sFS]\tConnect");
		exit(-1);
	}
	
	debugPrint1("[DS - sFS]\tExchanging messages with the front server at port %d.\n", (port + i - 1));
	
	int nbytes;	
	message m;
	memset(&m, 0, sizeof(message));
	
	m.flag = DS_SERVER_TAG;
	m.key = DS_port;
	debugPrint("[DS - fSH]\tSending the message with the data server tag and the port.\n");
	nbytes = send(front_sock_fd, &m, sizeof(message), 0);

	FS_connected = 1; 
}

// Checks if the front server is alive
void frontServerHandler(){

	int response = 0;

	while(1){
		if(FS_connected){
			response = heartbeat(front_sock_fd, 0, FIRST);
			if(response == DIE){
				debugPrint("[DS - fSH]\tThe front server has ordered a shutdown. Exiting.\n");
				quitDataServer();
			}
			else if(response == DEAD){
				debugPrint("[DS - fSH]\tThe front server has gone down. Attempting to restart it.\n");
				FS_connected = 0;
				if (!fork()){
					char current_DS_port[10];
					sprintf(current_DS_port, "%d", DS_port);
					char* FS_argv[] = {EXECUTE_FS, current_DS_port, 0};
					execve(FS_argv[0], FS_argv, NULL);
				}
			}
		}
		sleep(HEARTBEAT_TIME);
	}
}

void* clientHandler(void* pthread_arg){

	int client_fd;
	struct sockaddr_in client_addr;	
	int size_addr = sizeof(client_addr);	

	message m;
	int nbytes;	
	int port_msg;

	client_fd = accept(data_sock_fd,(struct sockaddr *)&client_addr, &size_addr);
	if(client_fd == -1){
		perror("Data Sv - Accept");
		exit(-1);
	}
	
	checkThreads();
	
	while ((nbytes = recv(client_fd, &m, sizeof(message), 0)) != 0){
		if (m.flag == FS_SERVER_TAG){
			if (FS_connected == 0){
				debugPrint1("[DS - cH]\tFront server reconnected. It's active and listenning to port %d.\n", m.key);
				port_msg = m.key;
				setupFrontServer(port_msg, 1);
			}
		}
		else{
			processRequest(client_fd, m, hashtable, log_fp, &log_lock);
		}
	}
	//printHashtable(hashtable);
	pthread_exit(NULL);
}

// If we have reached a critically low number of threads, create a new thread which single 
//	purpose is to create the necessary number of client handling threads.
void checkThreads(){
	
	pthread_t thread_creator_id;

	pthread_mutex_lock(&pool_lock);
	ready_count--;
	if (!creating_threads){
		if (ready_count <= MIN_THREADS){
			creating_threads = 1;
			pthread_create(&thread_creator_id, NULL, (void*) createThreads, (void*)(NULL));
			pthread_detach(thread_creator_id);
		}
	}
	pthread_mutex_unlock(&pool_lock);
}

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
	debugPrint1("[DS - cT]\tCreated a few threads! Ready count: %d\n", ready_count);
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
	backup();	
	freeHashtable(hashtable);
	fclose(log_fp);
	exit(0);
}
