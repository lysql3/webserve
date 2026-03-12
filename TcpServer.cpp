#include "TcpServer.hpp"
#include "ClientTable.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

TcpServer::TcpServer(int port) : _socket(port), _eventLoop(_socket, _table) {}

// TcpServer::~TcpServer() {
// 	if (_server_fd != -1) close(_server_fd);
// }
//
// void TcpServer::createSocket() {
// 	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
// 	_max_fd = _server_fd;
// 	if (_server_fd < 0) {
// 		std::cerr << "socket() failed\n";
// 		exit(EXIT_FAILURE);
// 	}
// }
//
// void TcpServer::configureSocket() {
// 	int opt = 1;
// 	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
// 				   sizeof(opt))) {
// 		std::cout << "setsocket(): " << std::strerror(errno) << std::endl;
// 		exit(EXIT_FAILURE);
// 	}
// }
//
// void TcpServer::configureAddress() {
// 	std::memset(&_addr, 0, sizeof(_addr));
//
// 	_addr.sin_family = AF_INET;
// 	_addr.sin_port = htons(_port);
// 	_addr.sin_addr.s_addr = INADDR_ANY;
// }
//
// void TcpServer::bindSocket() {
// 	if (bind(_server_fd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0) {
// 		std::cout << "bind(): " << std::strerror(errno) << std::endl;
// 		exit(EXIT_FAILURE);
// 	}
// }
//
// void TcpServer::startListening() {
// 	if (listen(_server_fd, 10) < 0) {
// 		std::cout << "listen(): " << std::strerror(errno) << std::endl;
// 		exit(EXIT_FAILURE);
// 	}
// }
//
// /* return new client fd or -1*/
// int TcpServer::acceptClient() {
// 	struct sockaddr_in client_addr;
// 	socklen_t len = sizeof(client_addr);
//
// 	int client_fd = accept(_server_fd, (struct sockaddr *)&client_addr, &len);
//
// 	if (client_fd < 0) {
// 		std::cout << "accept(): " << std::strerror(errno);
// 		return -1;
// 	}
//
// 	std::cout << "Client connected: ";
// 	std::cout << inet_ntoa(client_addr.sin_addr) << ":"
// 			  << ntohs(client_addr.sin_port);
// 	std::cout << ' ' << client_fd << std::endl;
//
// 	return client_fd;
// }

void TcpServer::init() {
	_socket.createSocket();
	_socket.configureSocket();
	_socket.configureAddress();
	_socket.bindSocket();
	_socket.startListening();
	std::cout << "Server initialized" << std::endl;
}

// void TcpServer::buildFdSets() {
// 	ClientMap &map = _table.getAll();
// 	FD_ZERO(&_rdset);
// 	FD_ZERO(&_wrset);
// 	for (ClientMap::iterator it = map.begin(); it != map.end(); ++it) {
// 		FD_SET(it->first, &_rdset);
// 		FD_SET(it->first, &_wrset);
// 	}
// 	FD_SET(_server_fd, &_rdset);
// }
//
// #include <fcntl.h>
//
// // NOTE: helper
// void make_non_blocking(int fd) {
// 	int flags = fcntl(fd, F_GETFL, 0);
// 	if (flags == -1) return;
//
// 	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
// }
//
// void TcpServer::handleNewConnection(int clientFd) {
// 	if (clientFd >= 0) {
// 		make_non_blocking(clientFd);
// 		_table.add(clientFd);
// 	}
// 	if (clientFd > _max_fd) _max_fd = clientFd;
// }
//
// bool TcpServer::handleClientActivity(int client_fd) {
// 	if (FD_ISSET(client_fd, &_rdset)) {
// 		if (!_table.get(client_fd)->onReadable()) {
// 			std::cout << client_fd << ": Disconnected\n";
// 			return false;
// 		}
// 	}
// 	if (FD_ISSET(client_fd, &_wrset)) {
// 		if (!_table.get(client_fd)->onWritable()) { return false; }
// 	}
//
// 	return true;
// }
//
// void TcpServer::processClients() {
// 	ClientMap &map = _table.getAll();
// 	for (ClientMap::iterator it = map.begin(); it != map.end();) {
// 		if (handleClientActivity(it->first) == false)
// 			it = _table.remove(it->first);
// 		else
// 			++it;
// 	}
// }

void TcpServer::run() {
	_eventLoop.setMaxfd(_socket.getFd());
	while (true) {
		_eventLoop.run();
	}
}
