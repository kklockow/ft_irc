
#pragma once

class Server
{
    private:
        int                 sockfd;
        int                 port;
        struct sockaddr_in  server_address;

    public:
        int                 get_sockfd();
        int                 get_port();
        struct sockaddr_in  get_server_address();
};