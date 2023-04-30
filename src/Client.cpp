#include "Client.hpp"

Client::Client(int clientSock, std::string& localIP, std::string& remoteIP, int remotePort) :
	clientSock(clientSock),
	localIP(localIP),
	remoteIP(remoteIP),
	remotePort(remotePort) {

	connection = new Connection(remoteIP, remotePort);
	connected = true;
	mode = RR_MODE;
	buffer_size = 0;
	ID = -1;
}

Client::~Client() {
	close(clientSock);
	delete connection;
	for (auto it = buffer.begin(); it != buffer.end(); ++it)
		delete it->second;
	buffer.clear();
}

bool 	Client::isConnected() const {return connected;}
bool 	Client::readyForRead() const {return mode == RR_MODE;}
bool	Client::readyForWrite() const {return mode == WR_MODE;}
bool	Client::readyToQueryServer() const {return mode == WQ_MODE;}
bool	Client::readyToReadServerResp() const {return mode == RQ_MODE;}
int		Client::getClientSocket() const {return clientSock;}
int 	Client::getRemoteSocket() const {return connection->getConnectionSocket();}
long	Client::getBufferSize() const {return buffer_size;}
const 	std::deque<std::pair<size_t, char *>>& Client::getBuffer() const {
	return buffer;
};

void 	Client::readRequest() {
	long 	len = 0;
	char	buff[BUFF_SIZE + 1];
	while ((len = recv(clientSock, buff, BUFF_SIZE, 0)) > 0) {
		char *dup = new char[len];
		memcpy(dup, buff, len);
		buffer.push_back(std::make_pair(len, dup));
		buffer_size += len;
		if (len < BUFF_SIZE)
			break;
	}
	if (len < 0) {
		mode = OFF_MODE;
		connected = false;
		throw ClientReadWriteException(strerror(errno));
	} else if (len == 0) {
		mode = OFF_MODE;
		connected = false;
	}
	else
		mode = WQ_MODE;
}
void 	Client::sendRequest() {
	long len = 0;
	while(buffer_size > 0 && !buffer.empty()) {
		len = connection->send(buffer.front().second, buffer.front().first);
		buffer_size -= len;
		if (len < (long) buffer.front().first)
			break;
		delete[] buffer.front().second;
		buffer.pop_front();
	}
	if (len < 0){
		mode = OFF_MODE;
		connected = false;
		throw Connection::ConnectionException(strerror(errno));
	}
	if (buffer_size > 0 && len > 0) {
		int size = buffer.front().first;
		memmove(buffer.front().second, buffer.front().second + len, size - len);
		buffer.front().first = size - len;
	}
	if (buffer_size == 0)
		mode = RQ_MODE;
}
void	Client::receiveResponse() {
	long 	len;
	char	buff[BUFF_SIZE + 1];
	while ((len = connection->receive(buff, BUFF_SIZE)) > 0) {
		buff[len] = 0;
		char *dup = new char[len];
		memcpy(dup, buff, len);
		buffer.push_back(std::make_pair(len, dup));
		buffer_size += len;
		if (len < BUFF_SIZE)
			break;
	}
	if (len < 0) {
		mode = OFF_MODE;
		connected = false;
		throw Connection::ConnectionException(strerror(errno));
	} else if (len == 0) {
		mode = OFF_MODE;
		connected = false;
	} else
		mode = WR_MODE;
}
void	Client::sendResponse() {
	long len = 0;
	while(buffer_size > 0 && !buffer.empty()) {
		len = send(clientSock, buffer.front().second, buffer.front().first, 0);
		buffer_size -= len;
		if (len < (long) buffer.front().first)
			break;
		delete[] buffer.front().second;
		buffer.pop_front();
	}
	if (len < 0){
		mode = OFF_MODE;
		connected = false;
		throw ClientReadWriteException(strerror(errno));
	}
	if (buffer_size > 0 && len > 0) {
		int size = buffer.front().first;
		memmove(buffer.front().second, buffer.front().second + len, size - len);
		buffer.front().first = size - len;
	}
	if (buffer_size == 0)
		mode = RR_MODE;
}


void Client::relay() {
	long 	lenr, lenw;
	char	buff[BUFF_SIZE + 1];
	while ((lenr = connection->receive(buff, BUFF_SIZE)) > 0) {
		if ((lenw = send(clientSock, buff, lenr, 0)) < 0)
			throw ClientReadWriteException(strerror(errno));
		// if len > len2 save the rest in the buffer and keep reading
		if (lenw < lenr) {
			std::cout << "buffering instead ..." << std::endl;
			char *dup = new char[lenr - lenw];
			memcpy(dup, buff + lenw, lenr - lenw);
			buffer.push_back(std::make_pair(lenr - lenw, dup));
			buffer_size += lenr - lenw;
			if (lenr < BUFF_SIZE)
				mode = WR_MODE;
			else
				receiveResponse();
			return;
		}
		if (lenr < BUFF_SIZE)
			break;
	}
	if (lenr < 0) {
		mode = OFF_MODE;
		connected = false;
		throw Connection::ConnectionException(strerror(errno));
	} else if (lenr == 0) {
		mode = OFF_MODE;
		connected = false;
	} else
		mode = RR_MODE;
}


std::string Client::getIP() const {return localIP;}
int Client::getID() const {return ID;}
void Client::setID(int id) {ID = id;}



Client::ClientReadWriteException::ClientReadWriteException():e("Error while reading/writing to client !"){}
Client::ClientReadWriteException::ClientReadWriteException(const char *e): e(e) {};
Client::ClientReadWriteException::~ClientReadWriteException() throw() {}
const char *Client::ClientReadWriteException::what() const throw() {return e.c_str();}