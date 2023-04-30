#ifndef __SERVER_HPP_
#define __SERVER_HPP_

#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <cstring>
#include <cstdio>
#include <iomanip>
#include <ctime>


#include "Client.hpp"
#include "IServer.hpp"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

class Client;

class ServerSelect : public IServer {

private:
	int 				servSock;
	int 				last_id;
	int 				max_fds;
	fd_set				rSet, wSet;
	std::list<Client *> clients;
	sockaddr_in 		servAddr;
	volatile bool 		looping;
	std::FILE			*logFile;

public:

	ServerSelect(std::string &localIp, int localPort, std::string &remoteIp, int remotePort, std::string &logPath);
	~ServerSelect() override;

	void init() override;
	void loop() override;
	void stop() override;

	class InitException: public std::exception {
	private:
		std::string e;
	public:
		InitException();
		InitException(const char* e);
		virtual ~InitException() throw();
		virtual const char *what() const throw();
	};

	class ProcessingException: public std::exception {
	private:
		std::string e;
	public:
		ProcessingException();
		ProcessingException(const char* e);
		ProcessingException(const std::exception& err);
		virtual ~ProcessingException() throw();
		virtual const char* what() const throw();
	};


private:
	void acceptNewClient();
	void resetSets();
	void clearDisconnected();
	void logRequest(Client *c);
};







#endif //__SERVER_HPP_