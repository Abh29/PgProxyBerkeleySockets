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
	Connection(std::string connIP, int connPort);
	~Connection();

	long send(const char *buff, size_t len) const;
	long receive(char *buff, size_t len) const;
	int  getConnectionSocket() const;


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