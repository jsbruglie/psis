#include "psiskv.h"
#include <pthread.h>
#include <signal.h>
#include <string.h>

#include "server_common.h"

int DS_port = -1;
int front_sock_fd;
int data_sv_sock_fd;
pthread_t connections_id;

// Function prototypes
void* dataServerHandler(void* thread_arg);
void* connectionHandler(void* thread_arg);
void consoleHandler(void);

void quitFrontServer(){
	close(front_sock_fd);
	printf("\n[FS - quit]\tSuccessfully closed all sockets.\n"); 
	exit(0);
}

void* dataServerHandler(void* thread_arg){
	
	printf("[FS - dSH]\tData Server Handler is up.\n");
	
	int hbeat = 1;
	while(1){	
		hbeat = heartbeat(data_sv_sock_fd, SECOND);
		if(hbeat != 0){
			DS_port = -1;
			printf("[FS - dSH]\tThe data server has gone down. Attempting to restart it.\n");

        	pthread_cancel(connections_id);
        	close(data_sv_sock_fd);
			
        	pthread_create(&connections_id, NULL, (void*) connectionHandler, (void*) NULL); // Create the connection handling socket
 			pthread_detach(connections_id);

			system(EXECUTE_DS);
		}
	}
}

void* connectionHandler(void* thread_arg){
	
	struct sockaddr_in client_addr;
	socklen_t size_addr = sizeof(client_addr);
	int nbytes;
	message m;
	pthread_t ds_handler_id;
	printf("[FS - cH]\tConnections Handler is up, waiting to receive the DS port fom the data server.\n");

	while(1){

		int client_fd = accept(front_sock_fd, (struct sockaddr *)&client_addr, &size_addr);
		nbytes = recv(client_fd, &m, sizeof(message), 0); //Use the value of m as confirmation
		if (DS_port == -1){
			if (m.flag == CLIENT_TAG){
				
				nbytes = send(client_fd, &DS_port, sizeof(DS_port), 0); 
				printf("[FS - dSS]\tClient tried to connect. Data server is still offline. Sending ERROR.\n");
				close(client_fd);
			}else if(m.flag == DS_SERVER_TAG){
				
				data_sv_sock_fd = client_fd;
				printf("[FS - cH]\tConnection from the data server!\n");
				// TODO - Maybe some sort of additional security, like a password or key? a bit like ssh
				int port;
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
			close(client_fd);
		}
	}
}

void consoleHandler(void){
	
	printf("[DS - Console]\ttype 'help' for help.\n");
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


int main(int argc, char **argv){

	// Configure CTR-C signal
	signal(SIGINT, quitFrontServer);

	// Configure server and client adress variables
	struct sockaddr_in front_server_addr, data_server_addr;
	socklen_t size_addr;	

	// Creates a socket for the front server
	front_sock_fd = createSocket(front_server_addr, FS_PORT);
	if (front_sock_fd == -1){
		exit(-1);
	}

	listen(front_sock_fd, MAX_CLIENTS);

 	// Creates a thread to handle both client and data server connections
 	pthread_create(&connections_id, NULL, (void*) connectionHandler, (void*) NULL); // Create the connection handling socket
 	pthread_detach(connections_id);

 	consoleHandler();	// Call the console handler

}