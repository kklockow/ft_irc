
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
        throw std::invalid_argument ("ERROR opening socket");
}

void Server::fill_socket_struct()
{
    int enabled = 1;

    bzero((char *) &this->_server_address, sizeof(this->_server_address));
    this->_server_address.sin_family = AF_INET;
    this->_server_address.sin_addr.s_addr = INADDR_ANY;
    this->_server_address.sin_port = htons(this->_port);
    if (setsockopt(this->_sockfd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) < 0)
        throw std::invalid_argument ("ERROR on setsockopt");
    if (fcntl(this->_sockfd, F_SETFL, O_NONBLOCK) < 0)
        throw std::invalid_argument ("ERROR on fcntl");
}

void Server::bind_server_address()
{
    if (bind(this->_sockfd, (struct sockaddr *) &this->_server_address, sizeof(this->_server_address)) < 0)
        throw std::invalid_argument ("ERROR on binding");
}

void Server::init_poll_struct(int fd)
{
    struct pollfd new_poll;

    memset(&new_poll, 0, sizeof(new_poll));
    new_poll.fd = fd;
    new_poll.events = POLLIN;
    new_poll.revents = 0;
    this->_poll_fd.emplace_back(new_poll);
}

void Server::init(char **av)
{
    this->create_socket();
    this->_port = std::atoi(av[1]);
	this->_password = av[2];
    this->fill_socket_struct();
    this->bind_server_address();
    listen(this->_sockfd, 5);
    this->init_poll_struct(this->_sockfd);
    this->running = true;
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
        throw std::invalid_argument ("ERROR on accept");
    this->_client.emplace_back(new_client);
    this->init_poll_struct(new_client.get_sockfd());
}

void Server::receive_data(int client_index)
{
    char    buffer[1028];
    int     n = 0;

    memset(&buffer, 0, sizeof(buffer));
    n = read(this->_client[client_index].get_sockfd(), buffer, 1028);
    if (n < 0)
        throw std::invalid_argument ("ERROR reading from socket");
    this->_client[client_index].set_last_message(buffer);
}

struct msg_tokens Server::parse_message_line(std::string line)
{
    std::stringstream           line_stream(line);
    std::string                 word;
    std::string                 trailing_substr;
    struct msg_tokens           tokenized_message;

    std::cout << "Line to be parsed: " << line << std::endl;
    if (!line.empty() && line[0] == ':')
        line_stream >> tokenized_message.prefix;
    line_stream >> tokenized_message.command;
    while (line_stream >> word)
    {
        if (word[0] == ':')
        {
            std::getline(line_stream, trailing_substr);
            tokenized_message.trailing = word.substr(1);
            tokenized_message.trailing.append(trailing_substr);
            break ;
        }
        tokenized_message.params.emplace_back(word);
    }
    return(tokenized_message);
}

void Server::commands_join(struct msg_tokens tokenized_message, int client_index)
{
    Channel new_channel;

    //loop through existing channels
    for (unsigned long i = 0; i < this->_channel.size(); i++)
    {
        //if channel already exists
        if (tokenized_message.params[0] == this->_channel[i].get_name())
        {
            //if client already in channel check still needed

            //adding client to channel list
            this->_channel[i].add_client_to_list(this->_client[client_index].get_nick_name());

            //message to client still missing
            return ;
        }
    }
    //set channelname to first param of message
    new_channel.set_name(tokenized_message.params[0]);
    //add client to client list of new channel
    new_channel.add_client_to_list(this->_client[client_index].get_nick_name());
    //add channel to channel vector
    this->_channel.emplace_back(new_channel);

    //proper message and error handling still needed
}

void Server::execute_command(struct msg_tokens tokenized_message, int client_index)
{
	if (!this->_client[client_index].get_authenticated() && tokenized_message.command != "PASS")
	{
		putstr_fd("ERROR: You must authenticate with PASS first\n", this->_client[client_index].get_sockfd());
		return;
	}
	else if (tokenized_message.command == "PASS")
	{
		authenticateClient(tokenized_message, client_index);
		return;
	}
    else if (tokenized_message.command == "JOIN")
    {
        this->commands_join(tokenized_message, client_index);
    }
	else if (tokenized_message.command == "STOP")
		this->running = false;
	else if (tokenized_message.command == "USER")
		putstr_fd(":server 001 newbie :Welcome to the IRC Network, newbie!~myuser@localhost\n", this->_client[client_index].get_sockfd());
	else if (tokenized_message.command == "QUIT")
	{
		putstr_fd("Goodbye!\n", this->_client[client_index].get_sockfd());
		close(this->_client[client_index].get_sockfd());
		// Remove the client and its pollfd.
		this->_client.erase(this->_client.begin() + client_index);
		this->_poll_fd.erase(this->_poll_fd.begin() + client_index + 1);
		return ;
	}
    else if (tokenized_message.command == "NICK")
    {
        this->_client[client_index].set_nick_name(tokenized_message.params[0]);
        putstr_fd(":server \n", this->_client[client_index].get_sockfd());
    }
    else
    {
        //code 421 is for command not found not sure about the syntax of the rest
        putstr_fd(":server 421 [", this->_client[client_index].get_sockfd());
        putstr_fd(tokenized_message.command, this->_client[client_index].get_sockfd());
        putstr_fd("] Command not found.\n", this->_client[client_index].get_sockfd());
    }
}

void Server::handle_data(int client_index)
{
	std::stringstream       message_stream(this->_client[client_index].get_last_message());
	std::string             line;
	struct msg_tokens       tokenized_message;

	while (std::getline(message_stream, line))
	{
		if (line.empty() || line.back() == '\n' || line.back() == '\r')
			line.pop_back();
		tokenized_message = this->parse_message_line(line);
		this->execute_command(tokenized_message, client_index);
	}
}

        // std::cout << "prefix: " << tokenized_message.prefix << std::endl;
        // std::cout << "command: " << tokenized_message.command << std::endl;
        // std::cout << "params: ";
        // for (auto &it : tokenized_message.params)
        // {
        //     std::cout << it << "-";
        // }
        // std::cout << std::endl;
        // std::cout << "trailing: " << tokenized_message.trailing << std::endl;

void Server::loop()
{
    while (this->running == true)
    {
        if (poll(&this->_poll_fd[0], this->_poll_fd.size(), -1) == -1)
            throw std::invalid_argument ("ERROR during poll");
        for (size_t i = 0; i < this->_poll_fd.size(); i++)
        {
            if (this->_poll_fd[i].revents & POLLIN && this->running == true)
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
    for (auto &it : this->_client)
    {
        close (it.get_sockfd());
    }
    this->_client.clear();

    for (auto &it : this->_poll_fd)
    {
        close (it.fd);
    }
    this->_poll_fd.clear();
}

bool Server::authenticateClient(const msg_tokens &tokenized_message, int client_index)
{
	std::string password;
	if (!tokenized_message.params.empty())
		password = tokenized_message.params[0];
	else
		password = tokenized_message.trailing;
	//std::cout << "Print password:" << password << std::endl;
	//std::cout << "Print server password:" << this->_password << std::endl;
	if (password != this->_password)
	{
		putstr_fd("ERROR: Password incorrect\n", this->_client[client_index].get_sockfd());
		close(this->_client[client_index].get_sockfd());
		this->_client.erase(this->_client.begin() + client_index);
		this->_poll_fd.erase(this->_poll_fd.begin() + client_index + 1);
		return false;
	}
	else
	{
		std::cout << "set authenticated(true)" << std::endl;
		this->_client[client_index].set_authenticated(true);
		return true;
	}
}
