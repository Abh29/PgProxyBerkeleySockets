#include <iostream>
#include <csignal>
#include "src/ServerSelect.hpp"


IServer *g_server;

void sigHandler(int sig) {
	std::cout << "signal " << sig << " received, stopping the server ..." << std::endl;
	g_server->stop();
}


int main(int argc, char** argv) {

	if (argc != 6) {
		std::cout << "./server localIP localPort remoteIP remotePort logPath" << std::endl;
		return 1;
	}

	signal(SIGTERM, sigHandler);
	signal(SIGINT, sigHandler);

	std::string localIP(argv[1]);
	std::string remoteIP(argv[3]);
	std::string logPath(argv[5]);
	int localPort = atoi(argv[2]);
	int remotePort = atoi(argv[4]);

	IServer *s = new ServerSelect(localIP, localPort, remoteIP, remotePort, logPath);
	g_server = s;
	try {
		std::cout << "init ..." << std::endl;
		s->init();
		std::cout << "loop ..." << std::endl;
		s->loop();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	delete s;
	std::cout << "bye!" << std::endl;
	return 0;
}
