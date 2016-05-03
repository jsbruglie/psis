/*For testing if the library works only*/

#include "psiskv.h"

#include <unistd.h>

int main(){
	
	char* val = "potato";
	char read_val[100];
	uint32_t key = 30;

	printf("Connecting to server\n");
	int kv_descriptor = kv_connect("127.0.0.1",9998); // This descriptor is what the API will use from now on

 	int ret = kv_write(kv_descriptor, key, val, strlen(val) + 1, 0);
 	printf("kv_write returned %d \n", ret);

	ret = kv_write(kv_descriptor, key, val, strlen(val) + 1, 0);
	printf("kv_write returned %d \n", ret);

	ret = kv_delete(kv_descriptor, key);
	printf("kv_delete returned %d \n", ret);
	
	ret = kv_delete(kv_descriptor, key);
	printf("kv_delete returned %d \n", ret);

	//sleep(5);

 	//printf("Reading from server\n");
 	//kv_read(kv_descriptor, key, read_val, strlen(read_val));
 	
 	printf("Closing connection to server\n");
 	kv_close(kv_descriptor);

	exit(0);
}
