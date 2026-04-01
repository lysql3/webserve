#include "EventLoop.hpp"

EventLoop::EventLoop(Socket &socket, ClientTable &table)
	: _socket(socket), _table(table) {
	_epollfd = epoll_create1(0);
	if (_epollfd == ERROR) exitError("epoll_create");
	epollAdd(_socket.getFd(), EPOLLIN);
}

EventLoop::~EventLoop() { close(_epollfd); }

void EventLoop::epollAdd(int fd, uint32_t events) {
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &ev) == ERROR)
		exitError("epoll_ctl: EPOLL_CTL_ADD");
}

void EventLoop::epollMod(int fd, u_int32_t events) {
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;
	if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &ev) == ERROR)
		exitError("epoll_ctl: EPOLL_CTL_MOD");
}

void EventLoop::disconnectClient(int fd) {
	if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL) == ERROR)
		exitError("epoll_ctl: EPOLL_CTL_DEL");
	_table.remove(fd);
	Logger::info("client " + to_stringg(fd) + ": disconnected");
}

bool EventLoop::handleStatus(int fd, ClientStatus status) {
	if (status == DISCONNECT) return false;
	else if (status == WANT_WRITE) epollMod(fd, EPOLLIN | EPOLLOUT);
	else if (status == DONE_WRITE) epollMod(fd, EPOLLIN);
	return true;
}

void EventLoop::processClients(struct epoll_event &ev) {
	int fd = ev.data.fd;
	Client *client = _table.get(fd);

	if (ev.events & EPOLLIN && !handleStatus(fd, client->onReadable())) {
		disconnectClient(fd);
		return;
	}
	if (ev.events & EPOLLOUT && !handleStatus(fd, client->onWritable())) {
		disconnectClient(fd);
		return;
	}
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

void EventLoop::loop() {
	int nfds;
	struct epoll_event events[MAX_EVENTS];

	while (true) {
		nfds = epoll_wait(_epollfd, events, MAX_EVENTS, ERROR /* time out */);
		if (nfds == ERROR) exitError("epoll_wait");
		for (int n = 0; n < nfds; ++n) {
			if (events[n].data.fd == _socket.getFd())
				handleNewConnections(_socket);
			else processClients(events[n]);
		}
	}
}
