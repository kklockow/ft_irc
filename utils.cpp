
#include "utils.hpp"

void error(std::string message, std::string type)
{
    if (type == "machine")
        std::cerr << message << " " << std::strerror(errno) << std::endl;
    if (type == "human")
        std::cerr << message << std::endl;
    exit(1);
}

int putstr_fd(char *str, int fd)
{
    int n = 0;

    for (size_t i = 0; i < strlen(str); i++)
        n += write(fd, &str[i], 1);
    return (n);
}
