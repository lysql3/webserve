#include "Client.hpp"
#include <unistd.h>
#include <iostream>
#include <string>

Client::Client(int socket_fd) : fd(socket_fd) {}

Client::~Client() {
	if (fd >= 0) close(fd);
}

bool Client::onReadable() {
	char buff[4096];
	int n = read(fd, buff, sizeof(buff));
	if (n == 0) return false;
	if (n == -1) return true;
	read_buffer.append(buff, n);
	write_buffer.append("Response\n");
	// while (true) {
	// 	consumed = tryParse(read_buffer, &write_buffer);
	// 	if (consuned == -1) break
	// 	read_buffer.erase(0, consumed);
	// }

	std::cout << fd << ": ";
	std::cout.write(buff, n);
	return true;
}

bool Client::onWritable() {
	if (hasDataToWrite()) {
		int n = write(fd, write_buffer.c_str(), write_buffer.length());
		if (n <= 0) return false;
		write_buffer.erase(0, n);
	}
	return true;
}

bool Client::hasDataToWrite() const { return !write_buffer.empty(); }
