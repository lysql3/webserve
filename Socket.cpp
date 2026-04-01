#include "Socket.hpp"

Socket::Socket(int port) : _port(port) {
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0) exitError("socket");
}

Socket::~Socket() {
	if (_server_fd != -1) close(_server_fd);
}
void make_non_blocking(int fd);
void Socket::configureSocket() {
	int opt = 1;

	make_non_blocking(_server_fd);
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
				   sizeof(opt)))
		exitError("setsocket");
}

void Socket::configureAddress() {
	std::memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_port);
	_addr.sin_addr.s_addr = INADDR_ANY;
}

void Socket::bindSocket() {
	if (bind(_server_fd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0)
		exitError("bind");
}

#define QUEUE_SIZE 10
void Socket::startListening() {
	if (listen(_server_fd, QUEUE_SIZE) < 0) exitError("listen");
	std::cout << "Listening on port: " << _port << std::endl;
}

int Socket::acceptClient() {
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	int client_fd = accept(_server_fd, (struct sockaddr *)&client_addr, &len);

	if (client_fd < 0) return -1;
	return client_fd;
}

int Socket::getFd() { return _server_fd; }
int Socket::getPort() { return _port; }
