#ifndef __SERVER_EPOLL_HPP_
#define __SERVER_EPOLL_HPP_

/*
 * using epoll for Linux as it tends to be faster than select.
 */

#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <list>
#include <cstring>
#include <cstdio>
#include <iomanip>
#include <ctime>

#include "Client.hpp"
#include "IServer.hpp"

#define MAX_EVENTS 64


class Client;

class ServerEpoll : public IServer {

private:

	int 								servSock;
	int 								last_id;
	std::unordered_map<int, Client *> 	fdClientMap;
	std::unordered_map<int, Client *> 	connClientMap;
	sockaddr_in 						servAddr;
	volatile bool 						looping;
	std::FILE							*logFile;
	int									epfd; // epoll instance fd
	epoll_event 						ev, ep_events[MAX_EVENTS]; // epoll events


public:
	ServerEpoll(std::string& localIp, int localPort, std::string& remoteIp, int remotePort, std::string& logPath);
	~ServerEpoll();

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
	void 	acceptNewClient();
	void 	clearDisconnected();
	size_t 	getCurrentTime(const char *format, char *buff, size_t len);
	void 	logRequest(Client *c);

};





#endif