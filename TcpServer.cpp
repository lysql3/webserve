#include "TcpServer.hpp"
#include <iostream>
#include "Logger.hpp"
#include "helper.hpp"

TcpServer::TcpServer(int port) : _socket(port), _eventLoop(_socket, _table) {}

void TcpServer::init() {
	_socket.configureSocket();
	_socket.configureAddress();
	_socket.bindSocket();
	_socket.startListening();
	Logger::info("Listeting on port: " + to_stringg(_socket.getPort()));
	Logger::info("Server initialized");
}

void TcpServer::run() {
	Logger::info("Server running");
	_eventLoop.loop();
}
