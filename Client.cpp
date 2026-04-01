#include "Client.hpp"

Client::Client(int socket_fd) : fd(socket_fd) {}

Client::~Client() {
	if (fd >= 0) close(fd);
}

ClientStatus Client::onReadable() {
	u_int8_t buff[4096];
	char response[200];
	strncpy(response, "Response\n", 10);
	int n = read(fd, buff, sizeof(buff));

	if (n == 0 || n == ERROR) return DISCONNECT;

	_rbuf.insert(_rbuf.end(), buff, buff + n);
	_wrbuf.insert(_wrbuf.end(), response, response + 10);

	// while (true) {
	// 	consumed = tryParse(read_buffer, &write_buffer);
	// 	if (consuned == -1) break
	// 	read_buffer.erase(0, consumed);
	// 	return WANT_WRITE;
	// }

	Logger::info("Client " + to_stringg(fd) + ": ");
	std::cout.write((char *)_rbuf.data(), _rbuf.size());
	return WANT_WRITE;
	// return OK;
}

ClientStatus Client::onWritable() {
	if (hasDataToWrite()) {
		int n = write(fd, _wrbuf.data(), _wrbuf.size());
		if (n <= 0) return DISCONNECT;
		_wrbuf.erase(_wrbuf.begin(), _wrbuf.begin() + n);
		if (hasDataToWrite()) return OK;
		return DONE_WRITE;
	}
	return OK;
}

bool Client::hasDataToWrite() const { return !_wrbuf.empty(); }
