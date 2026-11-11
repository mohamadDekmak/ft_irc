#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include <cerrno>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    char *end = NULL;
    errno = 0;
    long port = std::strtol(argv[1], &end, 10);

    if (errno == ERANGE || end == argv[1] || *end != '\0'
        || port < 1 || port > 65535)
    {
        std::cerr << "Error: port must be between 1 and 65535" << std::endl;
        return 1;
    }

    if (argv[2][0] == '\0')
    {
        std::cerr << "Error: password cannot be empty" << std::endl;
        return 1;
    }

    Server server(static_cast<int>(port), argv[2]);
    if (!server.createSocket() || !server.bindSocket()
        || !server.listenSocket())
        return 1;

    std::cout << "Listening on port " << port << std::endl;

    // Temporary startup test. C2-C3 will add the event loop.
    std::cout << "Press Enter to stop the server." << std::endl;
    std::cin.get();
    return 0;
}
