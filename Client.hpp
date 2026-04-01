#include <sys/types.h>
#include <unistd.h>
#include <vector>

enum ClientStatus {
	OK,			 // keep going, nothing special
	WANT_WRITE,	 // response ready, register EPOLLOUT
	DONE_WRITE,	 // buffer flushed, remove EPOLLOUT
	DISCONNECT	 // close the connection
};

class Client {
   private:
	int fd;

	std::vector<u_int8_t> _rbuf;
	std::vector<u_int8_t> _wrbuf;

   public:
	bool hasDataToWrite() const;  // maybe private
	Client(int socket_fd);

	~Client();

	ClientStatus onReadable();
	ClientStatus onWritable();
};
