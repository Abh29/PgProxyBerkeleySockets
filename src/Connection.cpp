#include "Connection.hpp"


Connection::Connection(std::string connIP, int connPort) :
	connIP(connIP),
	connPort(connPort)
	{
		connAddr.sin_port = htons(connPort);
		connAddr.sin_family = AF_INET;
		if (! inet_aton(connIP.c_str(), &connAddr.sin_addr))
			throw ConnectionException(strerror(errno));
		if ((connSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			throw ConnectionException(strerror(errno));
		int flags = fcntl(connSock, F_GETFD);
		flags |= O_NONBLOCK;
		fcntl(connSock, F_SETFD, flags);
		if (connect(connSock, (const sockaddr*) &connAddr, sizeof(connAddr)) < 0)
			throw ConnectionException(strerror(errno));
	};

Connection::~Connection() {
	close(connSock);
}

long Connection::send(const char *buff, size_t len) const {
	long out = ::send(connSock, buff, len, 0);
	return out;
};

long Connection::receive(char *buff, size_t len) const {
	long out = recv(connSock, buff, len, 0);
	return out;
}


int Connection::getConnectionSocket() const {return connSock;}


Connection::ConnectionException::ConnectionException() {
	e = "Error while initializing a Connection !";
}
Connection::ConnectionException::ConnectionException(const char *e) : e(e){}
Connection::ConnectionException::~ConnectionException() throw() {}

const char *Connection::ConnectionException::what() const throw() {
	return e.c_str();
}