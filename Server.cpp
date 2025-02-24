
#include "Server.hpp"

int Server::get_sockfd()
{
    return (this->_sockfd);
}

int Server::get_port()
{
    return (this->_port);
}

struct sockaddr_in Server::get_server_address()
{
    return (this->_server_address);
}

void Server::create_socket()
{
    this->_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_sockfd < 0)
        error("ERROR opening socket", "machine");
}

void Server::fill_socket_struct()
{
    bzero((char *) &this->_server_address, sizeof(this->_server_address));
    this->_server_address.sin_family = AF_INET;
    this->_server_address.sin_addr.s_addr = INADDR_ANY;
    this->_server_address.sin_port = htons(this->_port);
}

void Server::bind_server_address()
{
    if (bind(this->_sockfd, (struct sockaddr *) &this->_server_address, sizeof(this->_server_address)) < 0)
        error("ERROR on binding", "machine");
}

void Server::init(char **av)
{
    this->create_socket();
    this->_port = std::atoi(av[1]);
    this->fill_socket_struct();
    this->bind_server_address();
    listen(this->_sockfd, 5);
}

void Server::init_poll_struct()
{
    memset(&this->_s_poll, 0, sizeof(this->_s_poll));
    this->_s_poll.fd = this->_client.get_sockfd();
    this->_s_poll.events = POLLIN;
}

void Server::loop()
{
    int time;
    int n;
    char buffer[256];

    while (1)
    {
        time = 0;
        n = 0;
        this->_client.init(this->_sockfd);
        this->init_poll_struct();
        while (1)
        {
            if (poll(&this->_s_poll, 1, 100) == 1)
            {
                bzero(buffer,256);
                n = read(this->_client.get_sockfd(), buffer, 255);
                if (n < 0)
                    error("ERROR reading from socket", "machine");
                printf("Here is the message: %s\n", buffer);
                n = write(this->_client.get_sockfd(),"I got your message",18);
                if (n < 0) error("ERROR writing to socket", "machine");
                    break ;
            }
            else
                time++;
        }
        printf("Took %d   buffer is %s\n", time * 100, buffer);
        close(this->_client.get_sockfd());
        if (strncmp(buffer, "stop", 4) == 0)
            break ;
    }
}

void Server::end()
{
    close(this->_sockfd);
}