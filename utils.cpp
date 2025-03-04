
#include "utils.hpp"

// void error(std::string message, std::string type)
// {
//     if (type == "machine")
//         // std::cerr << message << " " << std::strerror(errno) << std::endl;
//     if (type == "human")
//         std::cerr << message << std::endl;
//     exit(1);
// }

int putstr_fd(std::string str, int fd)
{
    int len = str.size();
    int n;

    n = write(fd, str.c_str(), len);
    if (n != len)
    {
        std::cout << "lol somethings wrong" << std::endl;
    }
    return (n);
}
