#include "ServerEpoll.hpp"

ServerEpoll::ServerEpoll(std::string &localIp, int localPort, std::string &remoteIp, int remotePort,
						 std::string &logPath) : IServer(localIp, localPort, remoteIp, remotePort, logPath)
						 {
								std::cout << "Epoll server !" << std::endl;
								last_id = 0;
								memset(&servAddr, 0, sizeof(servAddr));
								looping = true;
								logFile = nullptr;
						 };

ServerEpoll::~ServerEpoll() {
	fclose(logFile);
}


void ServerEpoll::init() {

	// logfile
	if ((logFile = fopen(logPath.c_str(), "a")) == nullptr)
		throw InitException(strerror(errno));

	// listening socket
	servAddr.sin_port = htons(localPort);
	servAddr.sin_family = AF_INET;
	if (! inet_aton(localIP.c_str(), &servAddr.sin_addr))
		throw InitException((char *)"Invalid localIP address !\n");
	if ((servSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw InitException(strerror(errno));
	int flags = fcntl(servSock, F_GETFD);
	flags |= O_NONBLOCK;
	fcntl(servSock, F_SETFD, flags);
	if (bind(servSock, (const struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
		throw InitException(strerror(errno));
	if (listen(servSock, 0) < 0)
		throw InitException(strerror(errno));

	// epoll
	if ((epfd = epoll_create1(0)) < 0)
		throw InitException(strerror(errno));

	ev.events = EPOLLIN|EPOLLRDHUP|EPOLLERR|EPOLLHUP;
	ev.data.fd = servSock;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, servSock, &ev) < 0)
		throw InitException(strerror(errno));
}

void ServerEpoll::stop() {looping = false;}

void ServerEpoll::loop() {
	int nfds;

	while (looping) {

		if ((nfds = epoll_wait(epfd, ep_events, MAX_EVENTS, -1)) < 0)
			throw ProcessingException(strerror(errno));

		for (int i = 0; i < nfds; ++i) {
			if (ep_events[i].data.fd == servSock) {
				if ((ep_events[i].events & EPOLLIN) == EPOLLIN)
					acceptNewClient();
				else if ((ep_events[i].events & (EPOLLRDHUP|EPOLLERR|EPOLLHUP)) == (EPOLLRDHUP|EPOLLERR|EPOLLHUP))
					throw ProcessingException("Server socket error !");
			}
			else {
				if ((ep_events[i].events & EPOLLIN) == EPOLLIN) {
					std::unordered_map<int, Client *>::iterator it;
					int fd = ep_events[i].data.fd;
					if ((it = fdClientMap.find(fd)) != fdClientMap.end() && it->second->readyForRead()) {	// if event from client socket
						it->second->readRequest();
						logRequest(it->second);
					} else if ((it = connClientMap.find(fd)) != connClientMap.end() && it->second->readyToReadServerResp()) { // else if event from connection socket
						it->second->receiveResponse();
					}
				} else if ((ep_events[i].events & EPOLLOUT) == EPOLLOUT) {
					std::unordered_map<int, Client *>::iterator it;
					int fd = ep_events[i].data.fd;
					if ((it = fdClientMap.find(fd)) != fdClientMap.end() && it->second->readyForWrite()) {	// if event from client socket
						it->second->sendResponse();
					} else if ((it = connClientMap.find(fd)) != connClientMap.end() && it->second->readyToQueryServer()) { // else if event from connection socket
						it->second->sendRequest();
					}
				}
			}
		}
		clearDisconnected();
	}
}

void ServerEpoll::acceptNewClient() {
	sockaddr_in clt;
	unsigned int len = sizeof(clt);
	char c_ip[255] = {0};
	Client *c;

	// create a client
	int fd = accept(servSock, (sockaddr *) &clt, &len);
	if (fd < 0)
		throw InitException(strerror(errno));
	inet_ntop(AF_INET, &(clt.sin_addr), c_ip, 255);
	try {
		std::string ip(c_ip);
		c = new Client(fd, ip, remoteIP, remotePort);
		fdClientMap[c->getClientSocket()] = c;
		connClientMap[c->getRemoteSocket()] = c;
		c->setID(++last_id);
		std::cout << "client from address " << ip << " with id = " << c->getID() << " : is added" << std::endl;
	} catch (const Connection::ConnectionException& e) {
		std::cerr << "Could not opent a connection with the remote server!" << std::endl;
		throw ServerEpoll::ProcessingException(e.what());
	} catch (const std::exception&){
		throw ProcessingException((char *)"Could not add a new Client !");
	}

	// add fds to epoll set
	ev.events = EPOLLIN | EPOLLOUT;
	ev.data.fd = c->getClientSocket();
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, c->getClientSocket(), &ev) < 0)
		throw ProcessingException((char *)"Could not add the new client socket to the epoll set !");

	ev.events = EPOLLIN | EPOLLOUT;
	ev.data.fd = c->getRemoteSocket();
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, c->getRemoteSocket(), &ev) < 0)
		throw ProcessingException((char *)"Could not add the new connection socket to the epoll set !");

}

void ServerEpoll::clearDisconnected() {

	auto it = fdClientMap.begin();
	while (it != fdClientMap.end()) {
		Client *c = it->second;
		if (!c->isConnected()) {
			std::cout << "client from address " << c->getIP() << " with id = " <<
					c->getID() << " : is disconnected !" << std::endl;
			if (epoll_ctl(epfd, EPOLL_CTL_DEL, c->getClientSocket(), NULL) < 0)
				throw ProcessingException((char *)"Could not delete the client socket from the epoll set !");
			if (epoll_ctl(epfd, EPOLL_CTL_DEL, c->getRemoteSocket(), NULL) < 0)
				throw ProcessingException((char *)"Could not delete the connection socket from the epoll set !");
			auto p = it;
			++it;
			fdClientMap.erase(p);
			p = connClientMap.find(c->getRemoteSocket());
			connClientMap.erase(p);
			delete c;
		} else
			++it;
	}
}

size_t ServerEpoll::getCurrentTime(const char *format, char *buff, size_t len) {
	time_t _t;
	time(&_t);
	tm *_tm = localtime(&_t);
	strftime(buff, len, format, _tm);
	return strlen(buff);
}

void ServerEpoll::logRequest(Client *c) {
	char * tmp = c->getBuffer().front().second;
	if (c->getBufferSize() == 0 || *tmp != 'Q') return;

	char *logBuff = (char *)alloca(c->getBufferSize() + 1024);
	size_t timeLen = getCurrentTime("%Y.%m.%d %H:%M:%S\t-\t", logBuff, 50);
	size_t prefixLen = sprintf(logBuff + timeLen, "ip: %s\t-\tclient %d: ", c->getIP().c_str(), c->getID()) + timeLen;

	auto it = c->getBuffer().begin();
	memmove(logBuff + prefixLen, it->second + 5, it->first - 5);
	prefixLen += it->first - 5;
	while (++it != c->getBuffer().end()) {
		memmove(logBuff + prefixLen, it->second, it->first);
		prefixLen += it->first;
	}

	logBuff[prefixLen - 1] = 10;
	logBuff[prefixLen] = 0;

	if (fwrite(logBuff, sizeof(char), prefixLen, logFile) < prefixLen) {
		perror("Logging Error: ");
		std::cout << logBuff;
	}
}



ServerEpoll::InitException::InitException() {
	e = std::string("An Error occurred while initializing the server!");
}
ServerEpoll::InitException::InitException(const char* e) : e(e) {}

ServerEpoll::InitException::~InitException() throw() {};

const char *ServerEpoll::InitException::what() const throw() {
	return e.c_str();
}

ServerEpoll::ProcessingException::ProcessingException() {
	e = std::string("An Error occurred while running the server!");
}
ServerEpoll::ProcessingException::ProcessingException (const char* e) : e(e) {}

ServerEpoll::ProcessingException::~ProcessingException() throw() {};

const char *ServerEpoll::ProcessingException::what() const throw() {
	return e.c_str();
}


