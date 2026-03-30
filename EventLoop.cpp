#include "EventLoop.hpp"
#include <fcntl.h>
#include <cstdlib>
#include <iostream>

#include <sys/epoll.h>
#define MAX_EVENTS 128
// NOTE: helper

void make_non_blocking(int fd);
EventLoop::EventLoop(Socket &socket, ClientTable &table)
	: _socket(socket), _table(table) {}

void EventLoop::buildFdSets() {
	ClientMap &map = _table.getAll();
	FD_ZERO(&_rdset);
	FD_ZERO(&_wrset);
	for (ClientMap::iterator it = map.begin(); it != map.end(); ++it) {
		FD_SET(it->first, &_rdset);
		if (it->second->hasDataToWrite()) FD_SET(it->first, &_wrset);
	}
	FD_SET(_socket.getFd(), &_rdset);
}

void EventLoop::handleNewConnections(Socket &socket) {
	struct epoll_event ev;
	int clientFd;
	while ((clientFd = socket.acceptClient()) >= 0) {
		make_non_blocking(clientFd);
		_table.add(clientFd);
		ev.events = EPOLLIN;
		ev.data.fd = clientFd;
		if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, clientFd, &ev) == -1) {
			std::cerr << "epoll_ctl: conn_sock\n";
			exit(EXIT_FAILURE);
		}
	}
}

bool EventLoop::handleClientActivity(int client_fd) {
	if (FD_ISSET(client_fd, &_rdset))
		if (!_table.get(client_fd)->onReadable()) {
			std::cout << client_fd << ": Disconnected\n";
			return false;
		}
	if (FD_ISSET(client_fd, &_wrset))
		if (!_table.get(client_fd)->onWritable()) {
			std::cout << client_fd << ": Disconnected\n";
			return false;
		}
	return true;
}

void EventLoop::processClients(int clientFd) {
	ClientMap &map = _table.getAll();
	for (ClientMap::iterator it = map.begin(); it != map.end();) {
		if (handleClientActivity(it->first) == false)
			it = _table.remove(it->first);
		else
			++it;
	}
}

void EventLoop::loop() {
	struct epoll_event ev;
	struct epoll_event events[MAX_EVENTS];
	int nfds;
	_epollfd = epoll_create1(0);
	if (_epollfd == -1) {
		std::cerr << "epoll_create()\n";  // TODO: print_error_helper;
		std::exit(EXIT_FAILURE);
	}
	ev.events = EPOLLIN;
	ev.data.fd = _socket.getFd();
	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _socket.getFd(), &ev) == -1) {
		std::cerr << "epoll_ctl: listen_sock\n";
		exit(EXIT_FAILURE);
	}
	while (true) {
		nfds = epoll_wait(_epollfd, events, MAX_EVENTS, -1 /* time out */);
		if (nfds == -1) {
			std::cerr << "epoll_wait\n";
			exit(EXIT_FAILURE);
		}
		for (int n = 0; n < nfds; ++n) {
			if (events[n].data.fd == _socket.getFd()) {
				handleNewConnections(_socket);
			} else {
				processClients(events[n]);
			}
		}
	}
}
