
#include <iostream>
#include "utils.hpp"

void error(std::string message, std::string type)
{
    if (type == "machine")
        std::cerr << message << " " << std::strerror(errno) << std::endl;
    if (type == "human")
        std::cerr << message << std::endl;
    exit(1);
}