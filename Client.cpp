
#include "Client.hpp"

int Client::get_sockfd()
{
    return (this->_sockfd);
}

struct sockaddr_in Client::get_server_address()
{
    return (this->_client_address);
}

void Client::init(int server_sockfd)
{
    this->_client_len = sizeof(this->_client_address);
    this->_sockfd = accept(server_sockfd, (struct sockaddr *) &this->_client_address, &this->_client_len);
    if (this->_sockfd < 0)
        error("ERROR on accept", "machine");
}