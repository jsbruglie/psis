#include "psiskv.h"
#include <stdio.h>
#include <string.h>

#define MAX_VALUES 100

int main(int argc, char* argv[]){
	
	char linha[1000];
	int port;
	uint32_t i;

	if (argc > 1){
		port = atoi(argv[1]);
		if (port < 0){
			printf("Invalid port provided!\n");
			exit(-1);
		}
	}
	else{
		port = 9999;
	}
	int kv = kv_connect("127.0.0.1", port);

	printf("Writing kv-pair (i,i) with i from 0 to %d (overwrite not allowed).\n", MAX_VALUES - 1);
	for (i = 0; i < MAX_VALUES; i ++){
		sprintf(linha, "%u", i);
		kv_write(kv, i , linha, strlen(linha)+1, 0);
	}

	printf("Press enter to read values\n");
	getchar();
	for (i = 0; i < MAX_VALUES; i ++){
		if(kv_read(kv, i , linha, 1000) == 0){
			printf ("R1 - key - %10u value %s\n", i, linha);
		}
	}

	printf("Press enter to delete even values\n");
	getchar();
	for (i = 0; i < MAX_VALUES; i +=2){
		kv_delete(kv, i);
	}

	printf("Press enter to read values\n");
	getchar();
	for (i = 0; i < MAX_VALUES; i ++){
		if(kv_read(kv, i , linha, 1000) == 0){
			printf ("R2 - key - %10u value %s\n", i, linha);
		}
	}

	printf("Writing kv-pair (i,i*10) with i from 0 to %d (overwrite not allowed).\n", MAX_VALUES - 1);
	for (i = 0; i < MAX_VALUES; i ++){
		sprintf(linha, "%u", i*10);
		kv_write(kv, i , linha, strlen(linha)+1, 0);
	}

	printf("Press enter to read new values\n");
	getchar();
	for (i = 0; i < MAX_VALUES; i ++){
		if(kv_read(kv, i , linha, 1000) == 0){
			printf ("R3 - key - %10u value %s\n", i, linha);
		}
	}

	printf("Writing kv-pair (i,i*10) with i from 0 to %d (overwrite allowed).\n", MAX_VALUES - 1);
	for (i = 0; i < MAX_VALUES; i ++){
		sprintf(linha, "%u", i*10);
		kv_write(kv, i , linha, strlen(linha)+1, 1);
	}

	printf("Press enter to read new values\n");
	getchar();
	for (i = 0; i < MAX_VALUES; i ++){
		if(kv_read(kv, i , linha, 1000) == 0){
			printf ("R4 - key - %10u value %s\n", i, linha);
		}
	}
	kv_close(kv);
}
