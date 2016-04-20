#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "LinkedList.h"
#include "psiskv.h"

int processRequest(int new_fd, message m, LinkedList** lp);

int sv_write(int new_fd, message m, LinkedList** lp);

int sv_read(int new_fd, message m, LinkedList** lp);