#include <unistd.h>
#include <string>

class Client {
   public:
	int fd;

	std::string read_buffer;
	std::string write_buffer;

	Client(int socket_fd);

	~Client();

	bool hasDataToWrite() const;
};
