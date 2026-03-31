#include <sys/types.h>
#include <unistd.h>
#include <string>
enum ClientStatus {
    OK,           // keep going, nothing special
    WANT_WRITE,   // response ready, register EPOLLOUT
    DONE_WRITE,   // buffer flushed, remove EPOLLOUT
    DISCONNECT    // close the connection
}; // START HERE

class Client {
   public:
	int fd;

	std::string read_buffer;
	std::string write_buffer;

	Client(int socket_fd);

	~Client();

	bool onReadable();
	bool onWritable();
	bool hasDataToWrite() const;
};
