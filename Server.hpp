
#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <iostream>
#include "utils.hpp"
#include "Client.hpp"
#include <unistd.h>
#include <vector>
#include <fcntl.h>

class Server
{
    private:
        int                             _sockfd;
        int                             _port;
        struct sockaddr_in              _server_address;
        std::vector<Client>             _client;
        std::vector<struct pollfd>      _poll;
        void                            create_socket();
        void                            fill_socket_struct();
        void                            bind_server_address();
        void                            init_poll_struct(int fd);
        void                            accept_client();

    public:
        int                             get_sockfd();
        int                             get_port();
        struct sockaddr_in              get_server_address();
        void                            init(char **av);
        void                            loop();
        void                            end();
};