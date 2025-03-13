
#pragma once

#include <iostream>
#include <unistd.h>
#include <vector>

// void     error(std::string message, std::string type);
int      putstr_fd(std::string str, int fd);