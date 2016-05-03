#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "Hashtable.h"
#include "psiskv.h"

#define MAX_CLIENTS 10
#define HASHTABLE_SIZE 3 
#define MAX_BUFFER 1024

int processRequest(int client_fd, message m, Hashtable* _hashtable);

int sv_write(int client_fd, message m, Hashtable* _hashtable);

int sv_read(int client_fd, message m, Hashtable* _hashtable);

int sv_delete(int client_fd, message m, Hashtable* _hashtable);
