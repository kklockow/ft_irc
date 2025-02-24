#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "utils.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

class Client
{
    private:
        int                 _sockfd;
        struct sockaddr_in  _client_address;
        socklen_t           _client_len;

    public:
        int                 get_sockfd();
        struct sockaddr_in  get_server_address();
        void                init(int server_sockfd);
};