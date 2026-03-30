#include "TcpServer.hpp"
#include <iostream>

TcpServer::TcpServer(int port) : _socket(port), _eventLoop(_socket, _table) {}

void make_non_blocking(int fd);
void TcpServer::init() {
	_socket.createSocket();
	make_non_blocking(_socket.getFd());

	_socket.configureSocket();
	_socket.configureAddress();
	_socket.bindSocket();
	_socket.startListening();
	std::cout << "Server initialized" << std::endl;
}

void TcpServer::run() {
	std::cout << "Server running" << std::endl;
	_eventLoop.loop();
}
