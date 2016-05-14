/*For testing if the library works only*/

#include "psiskv.h"

#include <unistd.h>

int main(){
	
	char read_val[MAX_BUFFER];
	int ret;
	
	printf("Connecting to server\n");
	int kv_descriptor = kv_connect("127.0.0.1",9998); // This descriptor is what the API will use from now on

 	ret = kv_write(kv_descriptor, 1, "a", (strlen("a") + 1) * sizeof(char), 0);
 	printf("kv_write returned %d \n", ret);
	
	ret = kv_write(kv_descriptor, 2, "ab", (strlen("ab") + 1) * sizeof(char), 0);
 	printf("kv_write returned %d \n", ret);
	
	ret = kv_write(kv_descriptor, 3, "abc", (strlen("abc") + 1) * sizeof(char), 0);
	printf("kv_write returned %d \n", ret);

	ret = kv_delete(kv_descriptor, 2);
	printf("kv_delete returned %d \n", ret);
	
 	printf("Reading from server\n");
 	kv_read(kv_descriptor, 3, read_val, MAX_BUFFER);
 	
 	printf("Closing connection to server\n");
 	kv_close(kv_descriptor);

	exit(0);
}
