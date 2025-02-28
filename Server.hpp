
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
#include <sstream>
#include <regex>

class Server
{
    private:
        int                             _sockfd;
        int                             _port;
        struct sockaddr_in              _server_address;
        std::vector<Client>             _client;
        std::vector<struct pollfd>      _poll_fd;
        void                            create_socket();
        void                            fill_socket_struct();
        void                            bind_server_address();
        void                            init_poll_struct(int fd);
        void                            accept_client();
        void                            receive_data(int client_index);
        void                            handle_data(int client_index);
        void                            parse_message_line(std::string message_line, int client_index);

    public:
        int                             get_sockfd();
        int                             get_port();
        struct sockaddr_in              get_server_address();
        void                            init(char **av);
        void                            loop();
        void                            end();
};