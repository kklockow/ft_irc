
#include "Client.hpp"

void Client::set_sockfd(int new_sockfd)
{
    this->_sockfd = new_sockfd;
}

int Client::get_sockfd()
{
    return (this->_sockfd);
}

void Client::set_address(struct sockaddr_in new_sockaddr_in)
{
    this->_client_address = new_sockaddr_in;
}

struct sockaddr_in Client::get_address()
{
    return (this->_client_address);
}

void Client::set_len(socklen_t new_client_len)
{
    this->_client_len = new_client_len;
}

socklen_t Client::get_len()
{
    return (this->_client_len);
}

void Client::set_nick_name(char *new_name)
{
    std::string temp_name(new_name);

    this->_nick_name = temp_name;
}

std::string Client::get_nick_name()
{
    return(this->_nick_name);
}