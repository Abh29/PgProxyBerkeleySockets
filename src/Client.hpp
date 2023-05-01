#ifndef __CLIENT_HPP_
#define __CLIENT_HPP_

#include <deque>
#include <vector>

#include "Connection.hpp"
class Connection;

#define RR_MODE	0	// read request from the client
#define WR_MODE 1	// send response to client
#define RQ_MODE	2	// receive query response from the remote server
#define WQ_MODE	3	// send query to the remote server


#define OFF_MODE 5
#define BUFF_SIZE 8192


class Client {
private:
	int											clientSock;
	std::string									localIP;
	std::string									remoteIP;
	int											remotePort;
	bool										connected;
	int											mode;
	Connection									*connection;
	std::deque<std::pair<size_t, char *>> 		buffer;
	long 										buffer_size;
	int 										ID;

public:

	/*
	 * @param clientSock : the socket fd of the client
	 * @param localIP : the ip (ipv4) address of the client
	 * @param remoteIP : the ip (ipv4) address of the remote server
	 * @param remotePort : the port of the remote server
	 *
	 * create connection object and initializes private values,
	 * sets the mode to RR_MODE and the ID to -1
	 */
	Client(int clientSock, std::string& localIP, std::string& remoteIP, int remotePort);

	/*
	 * closes the client socket and delete the connection object
	 * also delete the buffer if there is still some content
	 */
	~Client();

	/*
	 * reads the request from the client through the client socket
	 * the request is then saved in the buffer and buffer_size is updated
	 * the mode is changed to WQ_MODE on success or OFF_MODE or fail
	 *
	 * throws ClientReadWriteException on error
	 */
	void readRequest();


	/*
	 * sends the content of the buffer back to the remote server through
	 * the connection->send() function, buffer_size is updated accordingly
	 * the mode is changed to RQ_MODE on success or OFF_MODE
	 *
	 * throws ClientReadWriteException on error
	 */
	void sendRequest();


	/*
	 * reads the response from the remote server through the connection->receive() function
	 * the response is then saved in the buffer and buffer_size is updated
	 * the mode is changed to WR_MODE on success or OFF_MODE or fail
	 *
	 * throws Connection::ConnectionException on error
	 */
	void receiveResponse();


	/*
	 * sends the content of the buffer back to the client through the client socket
	 * buffer_size is updated accordingly
	 * the mode is changed to RR_MODE on success or OFF_MODE
	 *
	 * throws ClientReadWriteException on error
	 */
	void sendResponse();

	/*
	 * reads the incoming response from the connection socket through
	 * connection->receive() then send it directly to the client through
	 * the client socket without saving to buffer.
	 * used when the client socket is ready for writing and the connection socket
	 * is ready for reading simultaneously
	 * in case of received data bigger then the sent data the rest is saved in buffer then
	 * receiveResponse() is called
	 *
	 * the mode is changed to RR_MODE if all the data is sent, or WR_MODE in case of buffering
	 * in the case of error OFF_MODE is set
	 *
	 * throws ClientReadWriteException or Connection::ConnectionException on error
	 */
	void relay();

	/*
	 * RR_MODE - read the request from the client
	 * returns true if mode == RR_MODE
	 */
	bool readyForRead() const;

	/*
	 * WR_MODE - write the response to the client
	 * returns true if mode == WR_MODE
	 */
	bool readyForWrite() const;

	/*
	 * WQ_MODE - write the query to the connection socket
	 * return true if mode == WQ_MODE
	 */
	bool readyToQueryServer() const;

	/*
	 * RQ_MODE - read the query response from the connection socket
	 * returns true if mode == RQ_MODE
	 */
	bool readyToReadServerResp() const;


	/*
	 * bool connected indicates that the client is still connected and running properly
	 * if the client or connection socket is closed connected = false
	 *
	 * returns connected
	 */
	bool isConnected() const;

	/*
	 * returns clientSock
	 */
	int	 getClientSocket() const;

	/*
	 * returns connection->getConnectionSocket()
	 */
	int  getRemoteSocket() const;

	/*
	 * buffer_size indicate the number of bytes saved from a reading (from the client or the server)
	 * buffer_size is the sum of all sizes of elements in the queue
	 *
	 * returns buffer_size
	 */
	long getBufferSize() const;

	/*
	 * buffer is queue of paris containing a chunk received as char* and it's size
	 *
	 * return buffer
	 */
	const std::deque<std::pair<size_t, char *>>& getBuffer() const;

	/*
	 * returns localIp  (the ip (ipv4) address of the client)
	 */
	std::string getIP() const;

	/*
	 * @param id: the id of the client
	 * sets the id of a client
	 */
	void setID(int id);

	/*
	 * returns ID (the id of the client)
	 */
	int getID() const;


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