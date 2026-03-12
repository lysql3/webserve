#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <netinet/in.h>
class Socket {
   private:
	int _server_fd;
	int _port;
	struct sockaddr_in _addr;

   public:
	Socket(int port);
	~Socket();

	void createSocket();
	void configureSocket();
	void configureAddress();
	void bindSocket();
	void startListening();
	int acceptClient();
	int getFd();
};
#endif
