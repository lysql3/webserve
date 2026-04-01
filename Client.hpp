#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include "Logger.hpp"
#include "helper.hpp"

enum ClientStatus { OK, WANT_WRITE, DONE_WRITE, DISCONNECT };

class Client {
   private:
	int fd;
	std::vector<u_int8_t> _rbuf;
	std::vector<u_int8_t> _wrbuf;
	bool hasDataToWrite() const;  // maybe private

   public:
	Client(int socket_fd);
	~Client();

	ClientStatus onReadable();
	ClientStatus onWritable();
};
