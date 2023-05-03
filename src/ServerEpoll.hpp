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
#include <map>
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
	std::map<char, std::string> 		messageTypes;


public:

	/*
	 * @param localIP : the ip (ipv4) address of the client
	 * @param localPort : the port of the local (proxy) server
	 * @param remoteIP : the ip (ipv4) address of the remote server
	 * @param remotePort : the port of the remote server
	 * @param logPath : the path to the logging file
	 *
	 * initializes looping to true and logFile to null
	 */
	ServerEpoll(std::string& localIp, int localPort, std::string& remoteIp, int remotePort, std::string& logPath);


	/*
	 * closes the logFile, close the server socket then it
	 * disconnects and removes the clients
	 */
	~ServerEpoll();


	/*
	 * opens the logging file with fopen, then open a listening socket (socket/bind/listen) and
	 * sets it to NONBLOCK.
	 * then it creates an epoll instance with epoll_create1 and adds the server socket to the
	 * epoll set of fds to be monitored
	 *
	 * throws InitException on error
	 */
	void init() override;


	/*
	 * works while looping == true
	 * uses epoll_wait with infinite time to wait for io event to occur on one of
	 * the fds monitored by epoll
	 * if servSock is ready for reading it accepts a new client
	 * then loops for each file descriptor in the ep_events and checks if it is a client or connection
	 * socket and accordingly to the client mode it performs a read/write to the client/remoteServer
	 * finally it deletes the disconnected clients
	 *
	 * throws ProcessingException on error.
	 */
	void loop() override;


	/*
	 * sets looping parameter to false
	 */
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

	/*
	 *	accepts a new connection to the server socket then creates a new client object
	 *	and adds it to the fdClientMap and connClientMap,
	 *	then it adds the client socket and the connection socket to the epoll set to be
	 *	monitored by epoll using epoll_ctl function, it monitors them for read and write
	 *	by setting the events mask to EPOLLIN | EPOLLOUT
	 *
	 *	throws ProcessingException or InitException on error.
	 */
	void 	acceptNewClient();

	/*
	 * loops through the clients list and deletes the disconnected client
	 * and removes them from the epoll set
	 */
	void 	clearDisconnected();


	/*
	 * @param format : the required format of the date-time to be passed to the strftime fuction
	 * @param buff : the buffer where the result will be saved
	 * @param len :  the size of the buffer
	 *
	 * creates a localtime instance then writes the date and time to the buffer using strftime
	 */
	size_t 	getCurrentTime(const char *format, char *buff, size_t len);


	/*
	 * @param c : a pointer to a client
	 *
	 * checks the first byte of the client buffer, if it equals 'Q' it saves the time
	 * and some basic information about the client to the log file using the fwrite function
	 * if it can not write to the file it outputs the logs to stdout
	 */
	void 	logRequest(Client *c);

	/*
	 * messageTypes contains a char which is the first bite of a received request from the
	 * client and maps it to a string used in logging
	 * this function fills messageTypes map with predefined char values and their meaning
	 * https://www.postgresql.org/docs/current/protocol-message-formats.html
	 * we can add more entries to this function to log other messages (commands, copy ...)
	 */
	void	fillMessageTypes();

};





#endif