#include "EventLoop.hpp"
#include <fcntl.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

// NOTE: helper

void make_non_blocking(int fd);
void EventLoop::epollAdd(int fd, uint32_t events) {
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		std::cerr << "epoll_ctl ADD: " << std::strerror(errno) << "\n";
		exit(EXIT_FAILURE);
	}
}

EventLoop::EventLoop(Socket &socket, ClientTable &table)
	: _socket(socket), _table(table) {
	_epollfd = epoll_create1(0);
	if (_epollfd == -1) {
		std::cerr << "epoll_create()\n";
		std::exit(EXIT_FAILURE);
	}
	epollAdd(_socket.getFd(), EPOLLIN);
}

void EventLoop::handleNewConnections(Socket &socket) {
	int clientFd;
	while ((clientFd = socket.acceptClient()) >= 0) {
		make_non_blocking(clientFd);
		_table.add(clientFd);
		epollAdd(clientFd, EPOLLIN);
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

void EventLoop::setWriteable(int fd, struct epoll_event &ev) {
	ev.events = ev.events | EPOLLOUT;
	if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
		std::cerr << "epoll_ctl\n";
		exit(EXIT_FAILURE);
	}
}

void EventLoop::processClients(struct epoll_event &ev) {
	int fd = ev.data.fd;
	Client *client = _table.get(fd);
	bool disconnect = false;

	if (_table.get(fd)->hasDataToWrite()) { setWriteable(fd, ev); }
	if (ev.events & EPOLLIN && !client->onReadable()) disconnect = true;
	if (ev.events & EPOLLOUT && !client->onWritable()) disconnect = true;
	if (disconnect) disconnectClient(fd);
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
