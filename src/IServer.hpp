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

	/*
	 * @param localIP : the ip (ipv4) address of the client
	 * @param localPort : the port of the local (proxy) server
	 * @param remoteIP : the ip (ipv4) address of the remote server
	 * @param remotePort : the port of the remote server
	 * @param logPath : the path to the logging file
	 */
	IServer(std::string& localIP, int localPort, std::string& remoteIP, int remotePort, std::string& logPath) :
	localIP(localIP),
	remoteIP(remoteIP),
	localPort(localPort),
	remotePort(remotePort),
	logPath(logPath)
	{};

	/*
	 * destructor for the server
	 */
	virtual ~IServer() = default;


	/*
	 * initializes the server
	 */
	virtual void init() = 0;

	/*
	 * the main loop of the server
	 */
	virtual void loop() = 0;

	/*
	 * stops the server's loop
	 */
	virtual void stop() = 0;

};

#endif // __I_SERVER_HPP_