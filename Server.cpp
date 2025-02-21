
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <iostream>
#include "Server.hpp"

int Server::get_sockfd()
{
    return (this->sockfd);
}

int Server::get_port()
{
    return (this->port);
}

struct sockaddr_in Server::get_server_address()
{
    return (this->server_address);
}