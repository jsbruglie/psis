# psis - Systems Programming course project
--------
The project consists in a key-value store that can be acessed by clients using the psiskv API. 

## Execution
In order to run, the following commands should be issued in a bash terminal, in order.

1. ``` make```
2. ``` ./front_server```
3. ``` ./data_server```

From this point on, both servers are running. A testing client application is provided. To use it, run 
 ```./client```. By default it will try to connect using port 9999. The port can also be specified: ```./client [port]```, where [port] is the desired port number. 

João Borrego & Pedro Abreu, 2016