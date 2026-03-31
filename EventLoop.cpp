#include "EventLoop.hpp"
#include <fcntl.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

// NOTE: helper

void make_non_blocking(int fd);
EventLoop::EventLoop(Socket &socket, ClientTable &table)
	: _socket(socket), _table(table) {
	struct epoll_event ev;
	_epollfd = epoll_create1(0);
	if (_epollfd == -1) {
		std::cerr << "epoll_create()\n";  // TODO: print_error_helper;
		std::exit(EXIT_FAILURE);
	}
	ev.events = EPOLLIN;
	ev.data.fd = _socket.getFd();
	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _socket.getFd(), &ev) == -1) {
		std::cerr << "epoll_ctl: listen_sock " << std::strerror(errno) << "\n";
		exit(EXIT_FAILURE);
	}
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

void EventLoop::disconnectClient(int fd) {
	std::cout << fd << ": Disconnected\n";
	if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
		std::cerr << "epoll_ctl: conn_sock\n";
		exit(EXIT_FAILURE);
	}
	_table.remove(fd);
}

void EventLoop::processClients(struct epoll_event &ev) {
	if (_table.get(ev.data.fd)->hasDataToWrite()) {
		ev.events = ev.events | EPOLLOUT;
		if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, ev.data.fd, &ev) == -1) {
			std::cerr << "epoll_ctl\n";
			exit(EXIT_FAILURE);
		}
	}
	if ((ev.events & EPOLLIN && !_table.get(ev.data.fd)->onReadable()) ||
		(ev.events & EPOLLOUT && !_table.get(ev.data.fd)->onWritable()))
		disconnectClient(ev.data.fd);
}

void EventLoop::loop() {
	int nfds;
	struct epoll_event events[MAX_EVENTS];

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
