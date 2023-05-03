#include "ServerSelect.hpp"


ServerSelect::ServerSelect(std::string &localIp, int localPort, std::string &remoteIp, int remotePort, std::string &logPath):
		IServer(localIp, localPort, remoteIp, remotePort, logPath),
		last_id(),
		max_fds()
		{
			std::cout << "Select server !" << std::endl;
			FD_ZERO(&rSet);
			FD_ZERO(&wSet);
			memset(&servAddr, 0, sizeof(servAddr));
			looping = true;
			logFile = nullptr;
		};

ServerSelect::~ServerSelect() {
	fclose(logFile);
	close(servSock);
	for (auto it = clients.begin(); it != clients.end(); ++it)
		delete *it;
	clients.clear();
};

void ServerSelect::init() {

	if ((logFile = fopen(logPath.c_str(), "a")) == nullptr)
		throw InitException(strerror(errno));

	servAddr.sin_port = htons(localPort);
	servAddr.sin_family = AF_INET;
	if (! inet_aton(localIP.c_str(), &servAddr.sin_addr))
		throw InitException((char *)"Invalid localIP address !\n");
	if ((servSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw InitException(strerror(errno));
	max_fds = servSock;
	int flags = fcntl(servSock, F_GETFD);
	flags |= O_NONBLOCK;
	fcntl(servSock, F_SETFD, flags);
	if (bind(servSock, (const struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
		throw InitException(strerror(errno));
	if (listen(servSock, 0) < 0)
		throw InitException(strerror(errno));

	fillMessageTypes();
}

void ServerSelect::loop() {
	int selected = 0;
	while (looping) {
		resetSets();
		if ((selected = select(max_fds + 1, &rSet, &wSet, NULL, NULL)) < 0)
			throw ProcessingException(strerror(errno));

		if (selected > 0) {
			if (FD_ISSET(servSock, &rSet) != 0)
				acceptNewClient();
			for (auto it = clients.begin(); it != clients.end(); ++it) {
				Client *c = *it;
				if (c->readyForRead() && FD_ISSET(c->getClientSocket(), &rSet)) {
					c->readRequest();
					logRequest(c);
				} else if (c->readyToQueryServer() && FD_ISSET(c->getRemoteSocket(), &wSet)) {
					c->sendRequest();
				}
				if (c->readyToReadServerResp() && FD_ISSET(c->getRemoteSocket(), &rSet) && FD_ISSET(c->getClientSocket(), &wSet)) {
					c->relay();
				} else if (c->readyToReadServerResp() && FD_ISSET(c->getRemoteSocket(), &rSet)) {
					c->receiveResponse();
				}else if (c->readyForWrite() && FD_ISSET(c->getClientSocket(), &wSet)) {
					c->sendResponse();
				}
			}
			clearDisconnected();
		}
	}
}

void ServerSelect::acceptNewClient() {
	sockaddr_in clt;
	unsigned int len = sizeof(clt);
	char c_ip[255] = {0};

	int fd = accept(servSock, (sockaddr *) &clt, &len);
	if (fd < 0)
		throw InitException(strerror(errno));
	inet_ntop(AF_INET, &(clt.sin_addr), c_ip, 255);
	try {
		std::string ip(c_ip);
		Client *c = new Client(fd, ip, remoteIP, remotePort);
		clients.push_back(c);
		c->setID(++last_id);
		std::cout << "client from address " << ip << " with id = " << c->getID() << " : is added" << std::endl;
	} catch (const Connection::ConnectionException& e) {
		std::cerr << "Could not opent a connection with the remote server!" << std::endl;
		throw ServerSelect::ProcessingException(e.what());
	} catch (const std::exception&){
		throw ProcessingException((char *)"Could not add a new Client !");
	}
};

void ServerSelect::resetSets() {

	FD_ZERO(&rSet);
	FD_ZERO(&wSet);
	FD_SET(servSock, &rSet);

	for (auto it = clients.begin(); it != clients.end(); ++it) {
		Client *c = *it;
		if (!c->isConnected()) continue;
		if (c->readyForRead()) {
			FD_SET(c->getClientSocket(), &rSet);
			max_fds = MAX(max_fds, c->getClientSocket());
		} else if (c->readyForWrite()) {
			FD_SET(c->getClientSocket(), &wSet);
			max_fds = MAX(max_fds, c->getClientSocket());
		} else if (c->readyToReadServerResp()) {
			FD_SET(c->getRemoteSocket(), &rSet);
			FD_SET(c->getClientSocket(), &wSet);
			max_fds = MAX(max_fds, c->getRemoteSocket());
		} else if (c->readyToQueryServer()) {
			FD_SET(c->getRemoteSocket(), &wSet);
			max_fds = MAX(max_fds, c->getRemoteSocket());
		}
	}
};

void ServerSelect::clearDisconnected() {
	auto it = clients.begin();
	while (it != clients.end()) {
		Client *c = *it;
		if (!c->isConnected()) {
			std::cout << "client from address " << c->getIP() << " with id = " <<
					  c->getID() << " : is disconnected !" << std::endl;
			delete *it;
			auto p = it;
			++it;
			clients.erase(p);
		} else
			++it;
	}
};

void ServerSelect::stop() {
	looping = false;
}

size_t ServerSelect::getCurrentTime(const char *format, char *buff, size_t len) {
	time_t _t;
	time(&_t);
	tm *_tm = localtime(&_t);
	strftime(buff, len, format, _tm);
	return strlen(buff);
}

void ServerSelect::logRequest(Client *c) {
	char * tmp = c->getBuffer().front().second;
	if (c->getBufferSize() == 0 || messageTypes.find((char) *tmp) == messageTypes.end()) return;

	char *logBuff = (char *)alloca(c->getBufferSize() + 1024);
	size_t timeLen = getCurrentTime("%Y.%m.%d %H:%M:%S\t-\t", logBuff, 50);
	size_t prefixLen = sprintf(logBuff + timeLen, "ip: %s\t-\tclient %d: (%s)\t", c->getIP().c_str(), c->getID(), messageTypes.find(*tmp)->second.c_str()) + timeLen;

	auto it = c->getBuffer().begin();
	memmove(logBuff + prefixLen, it->second + 5, it->first - 5);
	prefixLen += it->first - 5;
	while (++it != c->getBuffer().end()) {
		memmove(logBuff + prefixLen, it->second, it->first);
		prefixLen += it->first;
	}

	std::string seperator("\n*****************************\n\n");
	memmove(logBuff + prefixLen, seperator.c_str(), seperator.length());
	prefixLen += seperator.length();
	logBuff[prefixLen] = 0;

	if (fwrite(logBuff, sizeof(char), prefixLen, logFile) < prefixLen) {
		perror("Logging Error: ");
		std::cout << logBuff;
	}
}

void ServerSelect::fillMessageTypes() {
	messageTypes.insert(std::make_pair<char, std::string>('Q', "simple query"));
	messageTypes.insert(std::make_pair<char, std::string>('B', "extended query bind"));
	messageTypes.insert(std::make_pair<char, std::string>('P', "extended query parse"));
	messageTypes.insert(std::make_pair<char, std::string>('D', "extended query describe"));
}



ServerSelect::InitException::InitException() {
	e = std::string("An Error occurred while initializing the server!");
}
ServerSelect::InitException::InitException(const char* e) : e(e) {}

ServerSelect::InitException::~InitException() throw() {};

const char *ServerSelect::InitException::what() const throw() {
	return e.c_str();
}

ServerSelect::ProcessingException::ProcessingException() {
	e = std::string("An Error occurred while running the server!");
}
ServerSelect::ProcessingException::ProcessingException (const char* e) : e(e) {}

ServerSelect::ProcessingException::~ProcessingException() throw() {};

const char *ServerSelect::ProcessingException::what() const throw() {
	return e.c_str();
}
