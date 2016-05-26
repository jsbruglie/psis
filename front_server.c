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
int DS_port = -1;			// The port that DS is listening to
int FS_port = -1;			// The port that FS is listening to
int FS_shutdown = 0;		// Whether a shutdown has been commanded

int DS_connected = 0;

int front_sock_fd = -1;	
int data_sv_sock_fd = -1;

pthread_t ds_handler_id;

// Function prototypes
void setupDataServer();
void connectionsHandler();
void* dataServerHandler(void* thread_arg);

void* consoleHandler(void);
void shutdownAttempt();
void quitFrontServer();

// Main program
int main(int argc, char **argv){
	
	// Create a socket
	int i;
	for (i = 0; front_sock_fd == -1 && i < NUMBER_OF_TRIES; i++){
		if (DEFAULT_FS_PORT + i != DS_port)
			front_sock_fd = createSocket(DEFAULT_FS_PORT + i);
	}
	if (front_sock_fd == -1){
		exit(-1);
	}
	FS_port = DEFAULT_FS_PORT + i -1;

	// If the DS port was provided; Needed when recovering from a crash
	if (argc > 1){
		DS_port = atoi(argv[1]);
		setupDataServer();
	}

	listen(front_sock_fd, 5);

	// Configure CTR-C signal
	signal(SIGINT, shutdownAttempt);

	// Launch the console in a dedicated thread
	pthread_t console_id;
 	pthread_create(&console_id, NULL, (void*) consoleHandler, NULL);
 	pthread_detach(console_id);

 	// Handle incoming connections, both from client or the data server
 	connectionsHandler();
}

void setupDataServer(){
	
	printf("[FS - sDS]\tPort provided: %d. Front server is expectedly recovering from crash.\n", DS_port);
	if (DS_port < 0){
		debugPrint("[FS - sDS]\tIncorrect data server port provided. Exiting.\n");
		exit(-1);
	}

	struct sockaddr_in server_addr;
	int data_sv_sock_fd_tmp = socket(AF_INET, SOCK_STREAM, 0);
	
	if (data_sv_sock_fd_tmp == -1){
		perror("[FS - sDS]\tSocket");
		exit(-1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
	server_addr.sin_port = htons(DS_port);

	if (connect(data_sv_sock_fd_tmp, (const struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
		debugPrint("[FS - sDS]\tIncorrect data server port provided. Exiting.\n");
		exit(-1);
	}

	message m;
	memset(&m, 0, sizeof(message));
	m.flag = FS_SERVER_TAG;
	m.key = FS_port;	
	int nbytes = send(data_sv_sock_fd_tmp, &m, sizeof(message), 0);
	debugPrint("[FS - sDS]\tNotified the DS that the FS is alive again.\n");
	close(data_sv_sock_fd_tmp);
}

void connectionsHandler(){
	
	struct sockaddr_in client_addr;
	socklen_t size_addr = sizeof(client_addr);
	int client_fd;
	int nbytes;
	message m;
	int port_msg;

	debugPrint("[FS - cH]\tConnections Handler is up.\n");

	while(1){
		
		client_fd = accept(front_sock_fd, (struct sockaddr*)&client_addr, &size_addr);
		debugPrint("[FS - ch]\tConnection received.\n");
		
		nbytes = recv(client_fd, &m, sizeof(message), 0); // Receive a message with an id
		if (nbytes != 0){
			if (DS_connected == 0){
				if (m.flag == CLIENT_TAG){
					nbytes = send(client_fd, &DS_port, sizeof(DS_port), 0); 
					debugPrint("[FS - cH]\tClient tried to connect. Data server is still offline. Sending ERROR.\n");
				
				}else if(m.flag == DS_SERVER_TAG){
					data_sv_sock_fd = client_fd;
					DS_port = m.key;
					DS_connected = 1;
					printf("[FS - cH]\tReceived DS port %d.\n", DS_port);
					// Creates a thread that pings the front server
					pthread_create(&ds_handler_id, NULL, (void*) dataServerHandler, (void*) NULL);
					pthread_detach(ds_handler_id);
				}
			}
			else if(m.flag == CLIENT_TAG){
				
				// Send client the data port
				nbytes = send(client_fd, &DS_port, sizeof(DS_port), 0); 
				printf("[FS - cH]\tNew client %d. Sending the data port.\n", client_fd);	
			}
		}	
	}
}

void* dataServerHandler(void* thread_arg){
	
	debugPrint("[FS - dSH]\tData Server Handler is up.\n");
	
	int response;
	while(!FS_shutdown){	

		response = heartbeat(data_sv_sock_fd, ALIVE, SECOND);

		if(response == DEAD && DS_connected == 1){
			
			DS_connected = 0;
			debugPrint("[FS - dSH]\tThe data server has gone down. Attempting to restart it.\n");
			if (!fork()){
				char* DS_argv[] = {EXECUTE_DS, 0};
				execve(DS_argv[0], DS_argv, NULL);
			}
			pthread_exit(NULL);
		}
		sleep(HEARTBEAT_TIME);
	}
	heartbeat(data_sv_sock_fd, DIE, SECOND);
	quitFrontServer();
}


void* consoleHandler(void){
	
	debugPrint("[FS - Console]\ttype 'help' for help.\n");
	char command[100];

	while(1){
		if (fgets(command, 100, stdin) == NULL){
			// The program can't read input from the terminal, there is no use for the console
			pthread_exit(NULL);	
		}
		strtok(command, "\n");
		if(strcmp(command, "help") == 0){
			debugPrint("\thelp\tDisplays help.\n");
			debugPrint("\tquit \tCloses both the servers.\n");
		}else if(strcmp(command, "quit") == 0){
			shutdownAttempt();
		}
	}
}

void shutdownAttempt(){
	FS_shutdown = 1;
	debugPrint("[FS - sA]\tShutdown imminent.\n");
	if (DS_connected == 0)
		exit(0);
}

void quitFrontServer(){
	
	close(front_sock_fd);
	//close(data_sv_sock_fd);
	exit(0);
}