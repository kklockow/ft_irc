
#pragma once

#include <iostream>
#include <unistd.h>
#include <vector>


struct msg_tokens
{
    std::string                 prefix;
    std::string                 command;
    std::vector<std::string>    params;
    std::string                 trailing;
};


// void     error(std::string message, std::string type);
int      putstr_fd(std::string str, int fd);