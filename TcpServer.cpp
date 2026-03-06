#include "TcpServer.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

TcpServer::TcpServer(int port) : _server_fd(-1), _port(port) {}

TcpServer::~TcpServer() {
	if (_server_fd != -1) close(_server_fd);
}

void TcpServer::createSocket() {
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd < 0) {
		std::cerr << "socket() failed\n";
		exit(EXIT_FAILURE);
	}
}

void TcpServer::configureSocket() {
	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
				   sizeof(opt))) {
		std::cout << "setsocket(): " << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}

void TcpServer::configureAddress() {
	std::memset(&_addr, 0, sizeof(_addr));

	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_port);
	_addr.sin_addr.s_addr = INADDR_ANY;
}

void TcpServer::bindSocket() {
	if (bind(_server_fd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0) {
		std::cout << "bind(): " << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}

void TcpServer::startListening() {
	if (listen(_server_fd, 10) < 0) {
		std::cout << "listen(): " << std::strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
}

void TcpServer::start() {
	createSocket();
	configureSocket();
	configureAddress();
	bindSocket();
	startListening();

	std::cout << "Listening on port: " << _port << std::endl;
}

/* return new client fd or -1*/
int TcpServer::acceptClient() {
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	int client_fd = accept(_server_fd, (struct sockaddr *)&client_addr, &len);

	if (client_fd < 0) {
		std::cout << "accept(): " << std::strerror(errno);
		return -1;
	}

	std::cout << "Client connected: ";
	std::cout << inet_ntoa(client_addr.sin_addr) << ":"
			  << ntohs(client_addr.sin_port);
	std::cout << ' ' << client_fd << std::endl;

	return client_fd;
}
