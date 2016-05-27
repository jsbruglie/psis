#include "server_common.h"

// Verbose
//#define VERBOSE 1 // Uncomment this line for verbose terminal output

#ifdef VERBOSE
#define debugPrint(str){printf(str);}
#define debugPrint1(str,arg){printf(str,arg);}
#else
#define debugPrint(str)
#define debugPrint1(str,arg)
#endif

int createSocket(int port){

	struct sockaddr_in server_addr;

	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("[createSocket]\tSocket");
		return -1;
 	}

 	server_addr.sin_family = AF_INET;
    	server_addr.sin_addr.s_addr = INADDR_ANY;
   	 server_addr.sin_port = htons(port);

    	int err = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
 	if(err == -1){
		perror("[createSocket]\tBind");
		return -1;
	}
 	debugPrint1("[CreateSocket]\tSocket sucessfully created and binded. Port %d\n", port); // DEBUG
 	return sock_fd;
}

int heartbeat(int fd, int msg, int heartbeatPairID){
	
	int nbytes;
	int answer;

	if(heartbeatPairID == FIRST){
		//debugPrint("[heartbeat]\t1\n");
		nbytes = send(fd, &msg, sizeof(msg), 0);
		nbytes = recv(fd, &answer, sizeof(answer), 0);
		if(nbytes == 0){
			debugPrint("[heartbeat]\tNo response.\n");
			answer =  DEAD;
		}
	}
	else if(heartbeatPairID == SECOND){
		//debugPrint("[heartbeat]\t2\n");
		nbytes = recv(fd, &answer, sizeof(answer), 0);
		if(nbytes == 0){
			debugPrint("[heartbeat]\tNo response.\n");
			answer = DEAD;
		}
		else{
			nbytes = send(fd, &msg, sizeof(msg), 0);
		}
	}
	return answer;
}
