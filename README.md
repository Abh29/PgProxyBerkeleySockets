# PgProxyBerkleySockets
    - create a simple proxy server for postgresql database, the server logs the querries made to the database.

## implementation:
    - the code is devided as follows:
### Server:
    - Iserver.hpp is an interface defining the global shape of the server class:
        - a constructor that takes the local ip of the server and the DBserver and their 
            respective ports as well as the logging file's path
        - void init(); initialize the server (open socket, open log file ...)
        - void loop(); the server start listening to incomming trafic and delling with it
        - void stop(); to stop the server
    - I have added two implementations to this interface :
        - ServerSelect is an implemetation using the select syscall
        - ServerEpoll is an implementation using the epoll api for Linux
        - they are almost identical except for the part of testing io related events (readiness for reading writing ...)

### Client:
    - Each time a new incoming traffic to the server socket a new client object is added to the server
    - Each client creates a Connection object that allows it to communicate with the remote server
    - The client has a rotating logic with four steps (modes)
        1. the client sends a request to the proxy server
        2. the request is logged and sent to the remote server
        3. the remote server sends back a response
        4. the response is then sent back to the client
    - In each iteration of the server's loop the mode is checked and changed accordingly

### Connection:
    - A Connection object opens a connection with the database server through a socket, it has two main functions:
        - long send(const char *buff, size_t len) writes the content of the buffer to the connection socket
	    - long receive(char *buff, size_t len) reads the content of the connection socket to the buffer
        - They both return the number of read/write bytes

## Additionally:
    - The logging happens on the level of the server, since this is a response to a specific task where it is 
        required to log only the SQL-queries, for each incoming traffic from the client the first byte is checked
        if it equals 'Q' then the request is logged.

## Further Improvements:
    - For now the proxy server processes only one request at a time, which is not efficient at all !
        to solve this we could process each request in a separate thread (thread pool)
    - Implement a better solution for logging
    - Add more other request types to the logging (not only the SQL-querries)
    - ps: as this was a specific response to a task, it was explecitely said what could be used, this is 
        why i didn't implement the obove points.