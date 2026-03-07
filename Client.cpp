#include "Client.hpp"
#include <unistd.h>
#include <iostream>

Client::Client(int socket_fd) : fd(socket_fd) {}

Client::~Client() {
	if (fd >= 0) close(fd);
	std::cout  << fd << " closed\n";
}

bool Client::hasDataToWrite() const { return !write_buffer.empty(); }
