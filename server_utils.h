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

#define HASHTABLE_SIZE 59

#define BACKUP_FILE "backup.bin"
#define LOG_FILE "log.bin"

// Data server functions
int processRequest(int client_fd, message m, Hashtable* _hashtable, FILE* log_fp, pthread_mutex_t* log_lock);
int sv_read(int client_fd, message m, Hashtable* _hashtable);
int sv_write(int client_fd, message m, Hashtable* _hashtable, FILE* log_fp, pthread_mutex_t* log_lock);
int sv_delete(int client_fd, message m, Hashtable* _hashtable, FILE* log_fp, pthread_mutex_t* log_lock);

// Logging Functions

// Opens the log file and process each entry
FILE* processLogEntries(char* filename, Hashtable* _hashtable);

// Proccess a single log entry
int logEntry(FILE* log_fp, pthread_mutex_t* log_lock, int flag, int key, char* value, int value_length);

// Erases the log file, but keeps it open for writing
FILE* eraseLog(FILE* log_fp, char* file_name);
