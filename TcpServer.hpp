#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <netinet/in.h>
#include <string>

class TcpServer {
   private:
	int _server_fd;
	int _port;
	struct sockaddr_in _addr;

	void createSocket();
	void configureSocket();
	void configureAddress();
	void bindSocket();
	void startListening();
	int acceptClient();

   public:
	TcpServer(int port);
	~TcpServer();

	void init();
	void run();
};

#endif
