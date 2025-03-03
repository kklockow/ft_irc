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
        std::string         _nick_name;
        std::string         _last_message;


    public:
        void                set_sockfd(int new_sockfd);
        int                 get_sockfd();
        void                set_address(struct sockaddr_in new_sockaddr_in);
        struct sockaddr_in  get_address();
        void                set_len(socklen_t new_client_len);
        socklen_t           get_len();
        void                set_nick_name(char *new_name);
        std::string         get_nick_name();
        void                set_last_message(char *message);
        std::string         get_last_message();
};


