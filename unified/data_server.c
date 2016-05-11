#include "server_utils.h"
#include "server_common.h"

// Global hashtable
Hashtable* hashtable;

// Logging related variables
pthread_mutex_t log_lock;
FILE* log_fp;

// Socket global variables
int data_sock_fd;

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

	// Close and unlink socket
	close(data_sock_fd);
	printf("[DS - quit]\tSuccessfully closed socket.\n"); // DEBUG
	backup();	
	freeHashtable(hashtable);
	fclose(log_fp);
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
	client_fd = accept(data_sock_fd,(struct sockaddr*) &client_addr, &size_addr);
	if(client_fd == -1){
		perror("[DS - cH]\tAccept");
		// TODO - There is a slight possibility that there's an unexpected behaviour around here
		exit(-1);
	}
	
	while ((nbytes = recv(client_fd, &m, sizeof(message), 0)) != 0){
		printf("[DS - cH]\tRequest from client %d: FLAG %d KEY %d SIZE %d\n", client_fd, m.flag, m.key, m.value_length); 
		if((ret = processRequest(client_fd, m, hashtable, log_fp, &log_lock)) != 0){
			printf("[DS - cH]\tRequest was not fulfilled. Return value %d.\n", ret); //DEBUG
		}
		printHashtable(hashtable);
	}
	return NULL;
}

void* frontServerHandler(){

	struct sockaddr_in server_addr;	
	int err = -1;	

    // Open a socket
 	int fs_fd = socket(AF_INET, SOCK_STREAM, 0);
 	if (fs_fd == -1){
		perror("[DS - fSH]\tSocket");
		exit(-1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 		// Set destination IP number - localhost, 127.0.0.1 // 

	int i = 0;

	// Connect to the server
	do{
		server_addr.sin_port = htons(FS_PORT + i);
		err = connect(fs_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr));
		i++;
	}while(err == -1 && i < NUMBER_OF_TRIES);
	
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
	nbytes = send(fs_fd, &m, sizeof(message), 0);
	printf("[DSH - fSH]\tSending the data server port.\n");
	nbytes = send(fs_fd, &port, sizeof(port), 0);

	int hbeat = 1;

	while(1){
		hbeat = heartbeat(fs_fd, FIRST);
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
	
	// Configure server and client adress variables
	struct sockaddr_in server_addr, client_addr;
		
	// Creates a socket for the server
	data_sock_fd = createSocket(server_addr, DS_PORT);
 	printf("[DS - main]\tSocket created and binded. Data server ready to receive messages.\n"); // DEBUG

 	// Configure CTR-C signal
	signal(SIGINT, quitDataServer);

 	// Creates a thread to communicate with the front-server
 	pthread_t front_server; 
 	pthread_create(&front_server, NULL, (void*) frontServerHandler, NULL);
 	pthread_detach(front_server);

	listen(data_sock_fd, MAX_CLIENTS);

	while(1){
		clientHandler(NULL);
	}
}
