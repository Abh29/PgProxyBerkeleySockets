#ifndef __I_SERVER_HPP_
#define __I_SERVER_HPP_

#include <iostream>


class IServer {

protected:
	std::string localIP;
	std::string	remoteIP;
	int			localPort;
	int			remotePort;
	std::string logPath;

public:
	IServer(std::string& localIP, int localPort, std::string& remoteIP, int remotePort, std::string& logPath) :
	localIP(localIP),
	remoteIP(remoteIP),
	localPort(localPort),
	remotePort(remotePort),
	logPath(logPath)
	{};

	virtual ~IServer() = default;
	virtual void init() = 0;
	virtual void loop() = 0;
	virtual void stop() = 0;

};

#endif // __I_SERVER_HPP_