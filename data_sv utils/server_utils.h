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

int processRequest(int new_fd, message m, Hashtable* _hashtable);

int sv_write(int new_fd, message m, Hashtable* _hashtable);

int sv_read(int new_fd, message m, Hashtable* _hashtable);

int sv_delete(int new_fd, message m, Hashtable* _hashtable);