
#pragma once

#include <iostream>
#include <unistd.h>

void     error(std::string message, std::string type);
int      putstr_fd(char *str, int fd);