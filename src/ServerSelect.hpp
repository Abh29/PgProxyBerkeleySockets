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

	/*
	 * @param localIP : the ip (ipv4) address of the client
	 * @param localPort : the port of the local (proxy) server
	 * @param remoteIP : the ip (ipv4) address of the remote server
	 * @param remotePort : the port of the remote server
	 * @param logPath : the path to the logging file
	 *
	 * initializes the fd_sets, looping to true and logFile to null
	 */
	ServerSelect(std::string &localIp, int localPort, std::string &remoteIp, int remotePort, std::string &logPath);


	/*
	 * closes the logFile, close the server socket then it
	 * disconnects and removes the clients
	 */
	~ServerSelect() override;


	/*
	 * opens the logging file with fopen, then open a listening socket (socket/bind/listen) and
	 * sets it to NONBLOCK.
	 *
	 * throws InitException on error
	 */
	void init() override;


	/*
	 * works while looping == true
	 * uses select to check for fd that are ready for io
	 * if servSock is ready for reading it accepts a new client
	 * then loops for each client in the list of clients, and either reads or writes
	 * to the client or the remote server according to the mode of the client
	 * finally it deletes the disconnected clients
	 *
	 * throws ProcessingException on error.
	 */
	void loop() override;

	/*
	 * sets looping to false.
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
	 *	and adds it to the clients list, it sets the client socket to non-blocking
	 *
	 *	throws ProcessingException or InitException on error.
	 */
	void 	acceptNewClient();


	/*
	 * zeroes the fd_sets then fill them with the file descriptors of the clients or their
	 * connections accordingly to the client mode
	 */
	void 	resetSets();


	/*
	 * loops through the clients list and deletes the disconnected client
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
};







#endif //__SERVER_HPP_