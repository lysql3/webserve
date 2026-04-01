#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include "ClientTable.hpp"
#include "Socket.hpp"

#include <fcntl.h>
#include <sys/epoll.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include "Logger.hpp"
#include "helper.hpp"

#define MAX_EVENTS 128

class EventLoop {
   private:
	Socket &_socket;
	ClientTable &_table;
	int _epollfd;

	void handleNewConnections(Socket &socket);
	void processClients(struct epoll_event &ev);
	void disconnectClient(int fd);
	void epollMod(int fd, uint32_t events);
	void epollAdd(int fd, uint32_t events);
	bool handleStatus(int fd, ClientStatus status);

   public:
	EventLoop(Socket &socket, ClientTable &table);
	~EventLoop();
	void loop();
};

#endif
