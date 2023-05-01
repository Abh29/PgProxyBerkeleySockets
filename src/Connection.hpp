#ifndef __CONNECTION_HPP_
#define __CONNECTION_HPP_

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

class Connection {
private:
	std::string 	connIP;
	int				connPort;
	int 			connSock;
	sockaddr_in		connAddr;

public:

	/*
	 * @param connIP: is the ip (ipv4) of the remote server
	 * @param connPort: is the remote server port
	 *
	 * opens a socket to connect to the remote server
	 *
	 * throws ConnectionException on error
	 */
	Connection(std::string connIP, int connPort);

	/*
 	* close the socket
 	*/
	~Connection();

	/*
	 * @param buff: is the buffer which content is to be written
	 * @param len: is the length of bytes to be written
	 *
	 * tries to write len bytes from buff to the socket using send function
	 * from <sys/socket.h>
	 *
	 * returns the number of bytes sent, or -1 on error
	 */
	long send(const char *buff, size_t len) const;

	/*
	 * @param buff: is the buffer to which content is to be written
	 * @param len: is the maximum number to read from the socket
	 *
	 * tries to read len bytes from socket and write them to the buffer
	 * using recv function from <sys/socket.h>
	 *
	 * returns the number of bytes received, or -1 on error
	 */
	long receive(char *buff, size_t len) const;

	/*
	 * returns connSock
	 */
	int  getConnectionSocket() const;


	/*
	 * in case of error while reading/writing to the socket
	 * this exception is thrown.
	 */

	class ConnectionException: public std::exception {
	private:
		std::string e;
	public:
		ConnectionException();
		ConnectionException(const char* e);
		virtual ~ConnectionException() throw();
		virtual const char* what() const throw();
	};

};




#endif //__CONNECTION_HPP_