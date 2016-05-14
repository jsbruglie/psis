#include <pthread.h>
#include <signal.h>
#include <string.h>

#include "psiskv.h"
#include "server_common.h"

// Verbose
#define VERBOSE 1

#ifdef VERBOSE
#define debugPrint(str){printf(str);}
#else
#define debugPrint(str)
#endif

// Global variables
int DS_port = -1;
int FS_port = -1;
int front_sock_fd = -1;
int data_sv_sock_fd = -1;

pthread_t connections_id;
pthread_t ds_handler_id;

pthread_mutex_t connections_lock;

// Function prototypes
void* dataServerHandler(void* thread_arg);
void* connectionHandler(void* thread_arg);
void consoleHandler(void);

void quitFrontServer(){
	
	pthread_cancel(connections_id);
	// Intercept a heartbeat communication in order to order the shutdown of the data server
	pthread_mutex_lock(&connections_lock);
	heartbeat(data_sv_sock_fd, DIE, SECOND);
	DS_port = -1;
	pthread_mutex_unlock(&connections_lock);
	close(front_sock_fd);
	//close(data_sv_sock_fd);

	debugPrint("\n[FS - quit]\tSuccessfully closed all sockets.\n"); 
	exit(0);
}

void* dataServerHandler(void* thread_arg){
	
	debugPrint("[FS - dSH]\tData Server Handler is up.\n");
	
	int hbeat;
	while(1){	
		
		pthread_mutex_lock(&connections_lock);
		hbeat = heartbeat(data_sv_sock_fd, ALIVE, SECOND);
		pthread_mutex_unlock(&connections_lock);

		if(hbeat == DEAD && DS_port != -1){
			DS_port = -1;
			debugPrint("[FS - dSH]\tThe data server has gone down. Attempting to restart it.\n");

        	// Destroy the connection handling thread - prevents the program from hanging over a client request
        	pthread_cancel(connections_id);
        	close(data_sv_sock_fd);
			// Restart the connection handling thread
        	pthread_create(&connections_id, NULL, (void*) connectionHandler, (void*) NULL);
 			pthread_detach(connections_id);
 			// Restart the data server
			if (!fork()){
				char* argv[] = { EXECUTE_DS, 0 };
				execve(argv[0], argv, NULL);
			}
			pthread_exit(NULL);
		}
		sleep(HEARTBEAT_TIME);
	}
}

void* connectionHandler(void* thread_arg){
	
	struct sockaddr_in client_addr;
	socklen_t size_addr = sizeof(client_addr);
	int client_fd;
	int nbytes;
	message m;
	int port;
	debugPrint("[FS - cH]\tConnections Handler is up, waiting to receive the DS port fom the data server.\n");

	while((client_fd = accept(front_sock_fd, (struct sockaddr*)&client_addr, &size_addr)) != 0){

		nbytes = recv(client_fd, &m, sizeof(message), 0); // Receive a message with an id
		if (nbytes != 0){
		
			if (DS_port == -1){
				if (m.flag == CLIENT_TAG){
					
					nbytes = send(client_fd, &DS_port, sizeof(DS_port), 0); 
					debugPrint("[FS - dSS]\tClient tried to connect. Data server is still offline. Sending ERROR.\n");
				}else if(m.flag == DS_SERVER_TAG){
					
					data_sv_sock_fd = client_fd;
					debugPrint("[FS - cH]\tConnection from the data server!\n");
					nbytes = recv(data_sv_sock_fd, &port, sizeof(port), 0); //Receive a port from the dataserver
					DS_port = port; // Assign the port received to the DS_port
					printf("[FS - dSS]\tReceived DS port %d.\n", port);
					// Creates a thread that pings the front server
					pthread_create(&ds_handler_id, NULL, (void*) dataServerHandler, (void*) NULL);
					pthread_detach(ds_handler_id);
				}
			}
			else if(m.flag == CLIENT_TAG){
				
				// send client data port
				nbytes = send(client_fd, &DS_port, sizeof(DS_port), 0); 
				printf("[FS - cH]\tNew client %d. Sending the data port.\n", client_fd);	
			}
		}	
	}
}

void consoleHandler(void){
	
	debugPrint("[DS - Console]\ttype 'help' for help.\n");
	char command[100];
	int ret = 0;
	while(1){
		fgets(command, 100, stdin);
		strtok(command, "\n");
		if(strcmp(command, "help") == 0){
			printf("\thelp\tDisplays help.\n");
			printf("\tquit \tCloses both the servers.\n");
		}else if(strcmp(command, "quit") == 0){
			quitFrontServer();
		}
	}
}

int setupDataServer(){
	printf("[FS - sDS]\tPort provided: %d. Front server is expectedly recovering from crash.\n", DS_port);
	if (DS_port < 0){
		exit(-1);
	}
	struct sockaddr_in server_addr;

	int data_sv_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (data_sv_sock_fd == -1){
		perror("[FS - sDS]\tSocket");
		exit(-1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(DS_PORT);

	if (connect(data_sv_sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
		printf("[FS - sDS]\tIncorrect data server port provided. Exiting.\n");
		exit(-1);
	}

	message m;
	memset(&m, 0, sizeof(message));
	m.flag = FS_SERVER_TAG;	
	//m.key = FS_port;
	int nbytes = send(data_sv_sock_fd, &m, sizeof(message), 0);
}

int main(int argc, char **argv){

	
	int i = 0;

	// Creates a socket for the front server
	for (i = 0; front_sock_fd == -1 && i < NUMBER_OF_TRIES; i++){
		if (FS_PORT + i != DS_port)
			front_sock_fd = createSocket(FS_PORT + i);
	}
	if (front_sock_fd == -1){
		exit(-1);
	}
	FS_port = FS_PORT + i -1;

	if (argc > 1){
		DS_port = atoi(argv[1]);
		setupDataServer();
	}

	pthread_mutex_init(&connections_lock, NULL);

	listen(front_sock_fd, 5);

	// Configure CTR-C signal
	signal(SIGINT, quitFrontServer);

 	// Creates a thread to handle both client and data server connections
 	pthread_create(&connections_id, NULL, (void*) connectionHandler, (void*) NULL);
 	pthread_detach(connections_id);

 	consoleHandler();	// Call the console handler

}