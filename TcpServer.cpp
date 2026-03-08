#include "TcpServer.hpp"
#include "ClientTable.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <cerrno>
#include <cstdlib>
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

void TcpServer::init() {
	createSocket();
	configureSocket();
	configureAddress();
	bindSocket();
	startListening();

	std::cout << "Listening on port: " << _port << std::endl;
}

void build_rdset(ClientTable &table, fd_set &rdSet, int &maxFd) {
	ClientMap map = table.getAll();
	for (ClientMap::iterator it = map.begin(); it != map.end(); ++it) {
		FD_SET(it->first, &rdSet);
		if (it->first > maxFd) maxFd = it->first;
	}
}

void TcpServer::consumeServerSocket(fd_set &rdSet, ClientTable &table) {
	int clientFd;
	if (FD_ISSET(_server_fd, &rdSet)) {
		clientFd = acceptClient();
		if (clientFd >= 0) table.add(clientFd);
	}
}

void TcpServer::consumeClientSockets(fd_set &rdSet, ClientTable &table) {
	char buff[4064] = {0};
	ClientMap map = table.getAll();
	int n = 0;

	for (ClientMap::iterator it = map.begin(); it != map.end();) {
		if (FD_ISSET(it->first, &rdSet)) {
			n = read(it->first, buff, sizeof(buff));
			if (n == 0) {
				std::cout << it->first << " Disconnected" << "\n";
				table.remove(it->first);
				continue;
			} else {
				std::cout << it->first << " " << buff << "\n";
				bzero(buff, sizeof(buff));
			}
		}
		++it;
	}
}

void TcpServer::run() {
	fd_set rdSet;
	fd_set wrSet;
	ClientTable table;
	int maxFd;

	while (true) {
		// build read_set = listening + clients
		FD_ZERO(&wrSet);
		FD_ZERO(&rdSet);
		FD_SET(_server_fd, &rdSet);

		maxFd = _server_fd;
		build_rdset(table, rdSet, maxFd);
		select(maxFd + 1, &rdSet, &wrSet, NULL, NULL);
		consumeServerSocket(rdSet, table);
		consumeClientSockets(rdSet, table);
	}
}

/* return new client fd or -1*/
// TODO: move near private methods
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
