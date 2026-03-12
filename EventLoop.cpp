#include "EventLoop.hpp"
#include <fcntl.h>
#include <iostream>

// NOTE: helper
void make_non_blocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) return;

	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

EventLoop::EventLoop(Socket &socket, ClientTable &table)
	: _socket(socket), _table(table) {}

void EventLoop::buildFdSets() {
	ClientMap &map = _table.getAll();
	FD_ZERO(&_rdset);
	FD_ZERO(&_wrset);
	for (ClientMap::iterator it = map.begin(); it != map.end(); ++it) {
		FD_SET(it->first, &_rdset);
		FD_SET(it->first, &_wrset);
	}
	FD_SET(_socket.getFd(), &_rdset);
}

void EventLoop::handleNewConnection(int clientFd) {
	if (clientFd >= 0) {
		make_non_blocking(clientFd);
		_table.add(clientFd);
	}
	if (clientFd > _max_fd) _max_fd = clientFd;
}

bool EventLoop::handleClientActivity(int client_fd) {
	if (FD_ISSET(client_fd, &_rdset)) {
		if (!_table.get(client_fd)->onReadable()) {
			std::cout << client_fd << ": Disconnected\n";
			return false;
		}
	}
	if (FD_ISSET(client_fd, &_wrset)) {
		if (!_table.get(client_fd)->onWritable()) { return false; }
	}

	return true;
}

void EventLoop::processClients() {
	ClientMap &map = _table.getAll();
	for (ClientMap::iterator it = map.begin(); it != map.end();) {
		if (handleClientActivity(it->first) == false)
			it = _table.remove(it->first);
		else
			++it;
	}
}

void EventLoop::run() {
	std::cout << "MAX FD: " << _max_fd << std::endl;
	buildFdSets();
	select(_max_fd + 1, &_rdset, NULL, NULL, NULL);
	if (FD_ISSET(_socket.getFd(), &_rdset))
		handleNewConnection(_socket.acceptClient());
	processClients();
}

void EventLoop::setMaxfd(int fd) { _max_fd = fd; }
