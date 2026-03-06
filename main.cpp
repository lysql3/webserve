#include "TcpServer.hpp"

int main()
{
    TcpServer server(8080);

    server.start();
    server.acceptClient();

    return 0;
}
