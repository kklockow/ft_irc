
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

    new_client.set_nick_name((char *)"none");
    new_client.set_len(sizeof(new_client.get_address()));
    temp_client_len = new_client.get_len();
    new_client.set_sockfd(accept(this->_sockfd, (struct sockaddr *)&temp_client_address, &temp_client_len));
    if (new_client.get_sockfd() < 0)
        error("ERROR on accept", "machine");

    this->_client.push_back(new_client);
    this->init_poll_struct(new_client.get_sockfd());
}

// RFC 1459              Internet Relay Chat Protocol              May 1993


//            1. Pass message
//            2. Nick message
//            3. User message

// 4.1.1 Password message


//       Command: PASS
//    Parameters: <password>

//    The PASS command is used to set a 'connection password'.  The
//    password can and must be set before any attempt to register the
//    connection is made.  Currently this requires that clients send a PASS
//    command before sending the NICK/USER combination and servers *must*
//    send a PASS command before any SERVER command.  The password supplied
//    must match the one contained in the C/N lines (for servers) or I
//    lines (for clients).  It is possible to send multiple PASS commands
//    before registering but only the last one sent is used for
//    verification and it may not be changed once registered.  Numeric
//    Replies:

//            ERR_NEEDMOREPARAMS              ERR_ALREADYREGISTRED

//    Example:

//            PASS secretpasswordhere

// 4.1.2 Nick message

//       Command: NICK
//    Parameters: <nickname> [ <hopcount> ]

//    NICK message is used to give user a nickname or change the previous
//    one.  The <hopcount> parameter is only used by servers to indicate
//    how far away a nick is from its home server.  A local connection has
//    a hopcount of 0.  If supplied by a client, it must be ignored.

//    If a NICK message arrives at a server which already knows about an
//    identical nickname for another client, a nickname collision occurs.
//    As a result of a nickname collision, all instances of the nickname
//    are removed from the server's database, and a KILL command is issued
//    to remove the nickname from all other server's database. If the NICK
//    message causing the collision was a nickname change, then the
//    original (old) nick must be removed as well.

//    If the server recieves an identical NICK from a client which is
//    directly connected, it may issue an ERR_NICKCOLLISION to the local
//    client, drop the NICK command, and not generate any kills.


void Server::receive_data(int client_index)
{
    char    buffer[1028];
    int     n = 0;

    memset(&buffer, 0, sizeof(buffer));
    n = read(this->_client[client_index].get_sockfd(), buffer, 1028);
    if (n < 0)
        error("ERROR reading from socket", "machine");
    this->_client[client_index].set_last_message(buffer);
}

//maybe newline character to be sure whole message was send
    // n = putstr_fd((char *)"I got your message\n", fd);
    // if (n < 0)
    //     error("ERROR writing to socket", "machine");
    // n = 0;
    // n = putstr_fd((char *)":http://kvirc.net/ 001 newbie :Welcome to the Internet Relay Network newbie!\n", fd);
    // if (n < 0)
    //     error("ERROR writing to socket", "machine");
    // // printf("Here is the message: [%s]\n", buffer);
    // std::cout << "Message: [" << buffer << "]" << std::endl;
    // if (strncmp(buffer, "stop", 4) == 0)
    //     exit(0);
    // if (strncmp(buffer, "NICK ", 5) == 0)
    // {
    //     std::cout << "hi" << std::endl;
    //     this->_client[client_index].set_nick_name(buffer + 5);
    // }
    // n = putstr_fd(this->_client[client_index].get_name(), fd);
    // if (n < 0)
    //     error("ERROR writing to socket", "machine");

    // std::cout << "name is: [" << this->_client[client_index].get_nick_name() << "]" << std::endl;


// [":" <prefix> SPACE] <command> [params] [":" trailing]
void Server::parse_message_line(std::string line, int client_index)
{

}

void Server::handle_data(int client_index)
{
    std::stringstream    message_stream(this->_client[client_index].get_last_message());
    std::string          line;

    while (std::getline(message_stream, line))
    {
        if (line.empty() || line.back() == '\n')
            line.pop_back();
        this->parse_message_line(line, client_index);
    }
}

void Server::loop()
{
    while (1)
    {
        if (poll(&this->_poll_fd[0], this->_poll_fd.size(), -1) == -1)
            error("ERROR during poll", "machine");
        for (size_t i = 0; i < this->_poll_fd.size(); i++)
        {
            if (this->_poll_fd[i].revents & POLLIN)
            {
                if (i == 0)
                {
                    this->accept_client();
                }
                else
                {
                    this->receive_data(i - 1);
                    this->handle_data(i - 1);
                }
            }
        }
    }
}

void Server::end()
{
    close(this->_sockfd);
}