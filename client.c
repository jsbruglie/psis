/*For testing if the library works only*/

#include "psiskv.h"
// #include "config.h"

int main(){
    
    message m;
	
	int descriptor = kv_connect("127.0.0.1",9999);
	//This descriptor is what the API will use from now on
 
	exit(0);
}
