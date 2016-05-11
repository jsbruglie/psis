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

#define HASHTABLE_SIZE 3 

#define BACKUP_FILE "backup.bin"
#define LOG_FILE "log.bin"

int processRequest(int client_fd, message m, Hashtable* _hashtable, FILE* log_fp, pthread_mutex_t* log_lock);
int sv_read(int client_fd, message m, Hashtable* _hashtable);
int sv_write(int client_fd, message m, Hashtable* _hashtable, FILE* log_fp, pthread_mutex_t* log_lock);
int sv_delete(int client_fd, message m, Hashtable* _hashtable, FILE* log_fp, pthread_mutex_t* log_lock);

// Logging Functions

// Opens the log file and process each entry
FILE* processLogEntries(char* filename, Hashtable* _hashtable);
int logEntry(FILE* log_fp, pthread_mutex_t* log_lock, int flag, int key, char* value, int value_length);
FILE* eraseLog(FILE* log_fp, char* file_name);
