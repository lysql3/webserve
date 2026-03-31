#include "EventLoop.hpp"
#include <fcntl.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include "Logger.hpp"
#include "helper.hpp"
// NOTE: helper

template <typename T>
std::string to_stringg(T val);
void make_non_blocking(int fd);
void exitError(std::string msg);
#define ERROR -1

void EventLoop::epollAdd(int fd, uint32_t events) {
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ev) == ERROR)
		exitError("epoll_ctl: EPOLL_CTL_ADD");
}

EventLoop::EventLoop(Socket &socket, ClientTable &table)
	: _socket(socket), _table(table) {
	_epollfd = epoll_create1(0);
	if (_epollfd == ERROR) exitError("epoll_create");
	epollAdd(_socket.getFd(), EPOLLIN);
}

void EventLoop::handleNewConnections(Socket &socket) {
	int fd;
	while ((fd = socket.acceptClient()) >= 0) {
		make_non_blocking(fd);
		_table.add(fd);
		epollAdd(fd, EPOLLIN);
		Logger::info("client " + to_stringg(fd) + ": connected");
	}
}

void EventLoop::disconnectClient(int fd) {
	if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL) == ERROR)
		exitError("epoll_ctl: EPOLL_CTL_DEL");
	_table.remove(fd);
	Logger::info("client " + to_stringg(fd) + ": disconnected");
}

void EventLoop::epollMod(int fd, u_int32_t events) {
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &ev) == ERROR)
		exitError("epoll_ctl: EPOLL_CTL_MOD");
}

void EventLoop::processClients(struct epoll_event &ev) {
	int fd = ev.data.fd;
	Client *client = _table.get(fd);
	bool disconnect = false;

	if (ev.events & EPOLLIN && !client->onReadable()) disconnect = true;
	if (ev.events & EPOLLOUT && !client->onWritable()) disconnect = true;
	if (client->hasDataToWrite()) { epollMod(fd, EPOLLIN | EPOLLOUT); }
	if (!client->hasDataToWrite()) { epollMod(fd, EPOLLIN); }
	if (disconnect) disconnectClient(fd);
}

void EventLoop::loop() {
	int nfds;
	struct epoll_event events[MAX_EVENTS];

	while (true) {
		nfds = epoll_wait(_epollfd, events, MAX_EVENTS, ERROR /* time out */);
		if (nfds == ERROR) exitError("epoll_wait");
		for (int n = 0; n < nfds; ++n) {
			if (events[n].data.fd == _socket.getFd())
				handleNewConnections(_socket);
			else
				processClients(events[n]);
		}
	}
}
