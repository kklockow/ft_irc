
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
    int enabled = 1;

    bzero((char *) &this->_server_address, sizeof(this->_server_address));
    this->_server_address.sin_family = AF_INET;
    this->_server_address.sin_addr.s_addr = INADDR_ANY;
    this->_server_address.sin_port = htons(this->_port);
    if (setsockopt(this->_sockfd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) < 0)
        error("ERROR on setsockopt", "machine");
    if (fcntl(this->_sockfd, F_SETFL, O_NONBLOCK) < 0)
        error("ERROR on fcntl", "machine");
}

void Server::bind_server_address()
{
    if (bind(this->_sockfd, (struct sockaddr *) &this->_server_address, sizeof(this->_server_address)) < 0)
        error("ERROR on binding", "machine");
}

void Server::init_poll_struct(int fd)
{
    struct pollfd new_poll;

    memset(&new_poll, 0, sizeof(new_poll));
    new_poll.fd = fd;
    new_poll.events = POLLIN;
    new_poll.revents = 0;
    this->_poll_fd.push_back(new_poll);
}

void Server::init(char **av)
{
    this->create_socket();
    this->_port = std::atoi(av[1]);
    this->fill_socket_struct();
    this->bind_server_address();
    listen(this->_sockfd, 5);
    this->init_poll_struct(this->_sockfd);
}

void Server::accept_client()
{
    Client               new_client;
    struct sockaddr_in   temp_client_address;
    socklen_t            temp_client_len;

    new_client.set_len(sizeof(new_client.get_address()));
    temp_client_len = new_client.get_len();
    new_client.set_sockfd(accept(this->_sockfd, (struct sockaddr *)&temp_client_address, &temp_client_len));
    if (new_client.get_sockfd() < 0)
        error("ERROR on accept", "machine");
    this->_client.push_back(new_client);
    this->init_poll_struct(new_client.get_sockfd());
}

void Server::receive_data(int fd)
{
    char    buffer[1028];
    int     n;

    memset(&buffer, 0, sizeof(buffer));
    n = read(fd, buffer, 1028);
    if (n < 0)
        error("ERROR reading from socket", "machine");
    printf("Here is the message: %s\n", buffer);
    n = write(fd,"I got your message\n",19);
    if (n < 0)
        error("ERROR writing to socket", "machine");
    if (strncmp(buffer, "stop", 4) == 0)
        exit(0);
}

void Server::loop()
{
    while (1)
    {
        if (poll(&this->_poll_fd[0], this->_poll_fd.size(), -1) == -1)
            error("ERROR during poll", "machine");
        std::cout << this->_poll_fd.size() << std::endl;
        for (size_t i = 0; i < this->_poll_fd.size(); i++)
        {
            if (this->_poll_fd[i].revents & POLLIN)
            {
                if (i == 0)
                    this->accept_client();
                else
                    this->receive_data(this->_poll_fd[i].fd);
            }
        }
    }
}

void Server::end()
{
    close(this->_sockfd);
}