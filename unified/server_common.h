#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

// Heartbeat related macros (ping)
#define FIRST 0
#define SECOND 1
#define HEARTBEAT_TIME 2

#define MAX_CLIENTS 20

// Default ports
#define DS_PORT 9998
#define FS_PORT 9999

// Private data server id - distinguishes it from a client
#define DS_SERVER_TAG 14

// Bash command that launches the data server
#define EXECUTE_DS "./data_server"

// Creates a TCP socket and returns the respective file descriptor
int createSocket(struct sockaddr_in server_addr, int port); // TODO - ADD IP as parameters

// Implements a simple heartbeat protocol to determine if any of the servers have died
int heartbeat(int fd, int heartbeatPairID);