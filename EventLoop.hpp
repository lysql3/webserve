#ifndef EVENTLOOP_HPP
#define EVENTLOOP_HPP

#include "ClientTable.hpp"
#include "Socket.hpp"

#include <sys/epoll.h>
#define MAX_EVENTS 128

class EventLoop {
   private:
	Socket &_socket;
	ClientTable &_table;
	int _epollfd;

	// fd_set _rdset;
	// fd_set _wrset;
	// int _max_fd; // TODO: remove implementaiton

	// void buildFdSets();
	void handleNewConnections(Socket &socket);
	// bool handleClientActivity(int clientFd);
	void processClients(struct epoll_event &ev);
	void disconnectClient(int fd);
	void setWriteable(int fd, struct epoll_event &ev);
	void epollAdd(int fd, uint32_t events);

   public:
	EventLoop(Socket &socket, ClientTable &table);
	// void setMaxfd(int fd);
	void loop();
};

#endif
