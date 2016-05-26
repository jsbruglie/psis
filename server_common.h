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

#include <fcntl.h>
#include <sys/stat.h>

// Heartbeat related macros
#define FIRST 0
#define SECOND 1
#define ALIVE 0
#define DEAD -1
#define DIE -2
#define HEARTBEAT_TIME 2 // in seconds

// Default server ports
#define DEFAULT_FS_PORT 9999
#define DEFAULT_DS_PORT 10000

// Localhost IP
#define LOCALHOST "127.0.0.1"

// Private front and data server id - distinguishes them from clients
#define FS_SERVER_TAG 7328827 // random integer
#define DS_SERVER_TAG 5349297 // random integer

// Shell command that launches the servers
#define EXECUTE_FS "./front_server"
#define EXECUTE_DS "./data_server"

// Creates a TCP socket and returns the respective file descriptor
int createSocket(int port);

// Implements a simple heartbeat protocol to determine if any of the servers have died or order shutdown
int heartbeat(int fd, int msg, int heartbeatPairID);