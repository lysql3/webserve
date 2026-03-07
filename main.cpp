#include "TcpServer.hpp"

int main()
{
    TcpServer server(8080);

    server.init();
	server.run();
    // server.acceptClient();

    return 0;
}
