#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <climits>

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

    // Server startup will be added in C1.
    std::cout << "Valid port: " << port << std::endl;
    return 0;
}
