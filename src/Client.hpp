#ifndef __CLIENT_HPP_
#define __CLIENT_HPP_

#include <deque>
#include <vector>

#include "Connection.hpp"
class Connection;

#define RR_MODE	0	// read request from the client
#define WR_MODE 1   // send response to client
#define RQ_MODE	2	// receive query response from the remote server
#define WQ_MODE	3	// send query to the remote server


#define OFF_MODE 5
#define BUFF_SIZE 8192


class Client {
private:
	int								clientSock;
	std::string						localIP;
	std::string						remoteIP;
	int								remotePort;
	bool							connected;
	int								mode;
	Connection						*connection;
	std::deque<std::pair<size_t, char *>> 		buffer;
	long 							buffer_size;
	int 							ID;

public:
	Client(int clientSock, std::string& localIP, std::string& remoteIP, int remotePort);
	~Client();

	bool isConnected() const;
	bool readyForRead() const;
	bool readyForWrite() const;
	bool readyToQueryServer() const;
	bool readyToReadServerResp() const;
	void readRequest(); 	//read request from client
	void sendRequest();		//send request to the remote server
	void receiveResponse();	//read response from the remote server
	void sendResponse();	//send response to the client
	int	 getClientSocket() const;
	int  getRemoteSocket() const;
	long getBufferSize() const;
	const std::deque<std::pair<size_t, char *>>& getBuffer() const;
	std::string getIP() const;
	void setID(int id);
	int getID() const;

	void relay();


	class ClientReadWriteException: public std::exception {
	private:
		std::string e;
	public:
		ClientReadWriteException();
		ClientReadWriteException(const char* e);
		virtual ~ClientReadWriteException() throw();
		virtual const char* what() const throw();
	};

};

#endif //__CLIENT_HPP_