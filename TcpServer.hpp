#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include "ClientTable.hpp"
#include "EventLoop.hpp"

#include <iostream>
#include "Logger.hpp"
#include "Socket.hpp"
#include "helper.hpp"

class TcpServer {
   private:
	Socket _socket;
	ClientTable _table;
	EventLoop _eventLoop;

   public:
	TcpServer(int port);

	void init();
	void run();
};

#endif
