
#include "Server.hpp"

Server::msg_tokens Server::error_message(std::string error_code, std::string message)
{
    msg_tokens error_message;
    error_message.command = error_code;
    error_message.params.push_back(message);
    return (error_message);
}

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


//not a fatal error should not quit whole server
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

Server::msg_tokens Server::parse_message_line(std::string line)
{
    std::stringstream           line_stream(line);
    std::string                 word;
    std::string                 trailing_substr;
    struct msg_tokens           tokenized_message;

	if (line.empty())
	    return (error_message("421", "Unknown"));
    if (line[0] == ':')
        line_stream >> tokenized_message.prefix;
    if (!(line_stream >> tokenized_message.command))
		return (error_message("421", "Unknown"));
    while (line_stream >> word)
    {
        if (word[0] == ':')
        {
            std::getline(line_stream, trailing_substr);
            tokenized_message.trailing = word.substr(1) + trailing_substr;
            break ;
        }
        tokenized_message.params.emplace_back(word);
    }
    return (tokenized_message);
}

int Server::get_client_index_through_name(std::string client_name)
{
    for (unsigned int i = 0; i < this->_client.size(); i++)
    {
        if (client_name == this->_client[i].get_nick_name())
            return(i);
    }
    return (-1);
}

int Server::get_channel_index_through_name(std::string channel_name)
{
    for (unsigned int i = 0; i < this->_channel.size(); i++)
    {
        if (channel_name == this->_channel[i].get_name())
            return (i);
    }
    return (-1);
}

void Server::commands_join_message_clients(std::string channel_name)
{
    int                         channel_index = 0;
    std::vector<std::string>    client_list;
    int                         current_client_index = 0;

    channel_index = this->get_channel_index_through_name(channel_name);
    client_list = this->_channel[channel_index].get_client_list();
    for (unsigned int i = 0; i < client_list.size(); i++)
    {
        current_client_index = get_client_index_through_name(client_list[i]);

        std::string message_to_clients =    ":"
                                            + this->_client[current_client_index].get_nick_name()
                                            + "!"
                                            + this->_client[current_client_index].get_user_name()
                                            + "@localhost JOIN "
                                            + channel_name
                                            + "\n";
        putstr_fd(message_to_clients, this->_client[current_client_index ].get_sockfd());
    }
}

void Server::commands_join(struct msg_tokens tokenized_message, int client_index)
{
    Channel                     new_channel;
    int                         channel_index;
    std::vector<std::string>    client_list;


    if (tokenized_message.params.empty() || tokenized_message.params[0].empty())
    {
        putstr_fd(":server 461 JOIN :Not enough parameters\n", this->_client[client_index].get_sockfd());
        return ;
    }

    channel_index = this->get_channel_index_through_name(tokenized_message.params[0]);
    if (channel_index != -1)
    {
        client_list = this->_channel[channel_index].get_client_list();
        for (unsigned int i = 0; i < client_list.size(); i++)
        {
            if (client_list[i] == this->_client[client_index].get_nick_name())
            {
                putstr_fd(":server 443 " + this->_client[client_index].get_nick_name() + " "
                        + tokenized_message.params[0] + " :You're already in this channel\n",
                        this->_client[client_index].get_sockfd());
                return ;
            }
        }
        this->_channel[channel_index].add_client_to_list(this->_client[client_index].get_nick_name());
    }
    else
    {
        //set channelname to first param of message
        new_channel.set_name(tokenized_message.params[0]);
        //add client to client list of new channel
        new_channel.add_client_to_list(this->_client[client_index].get_nick_name());
        //add channel to channel vector
        this->_channel.emplace_back(new_channel);
        channel_index = this->_channel.size() - 1;
    }
    this->commands_join_message_clients(tokenized_message.params[0]);
    //will putstr channel description later or error when no channeldescription
    std::string description_message =   ":server 332 "
                                        + this->_client[client_index].get_nick_name()
                                        + " "
                                        + tokenized_message.params[0]
                                        + " :Hi bin die description\n";

    putstr_fd(description_message, this->_client[client_index].get_sockfd());

    // write a list of all clients in channel to new joined client
    std::string client_list_message =   ":server 353 "
                                        + this->_client[client_index].get_nick_name()
                                        + " = "
                                        + tokenized_message.params[0]
                                        + " :";

    client_list = this->_channel[channel_index].get_client_list();
    for (unsigned int i = 0; i < client_list.size(); i++)
        client_list_message += client_list[i] + " ";
    client_list_message += "\n";

    putstr_fd(client_list_message, this->_client[client_index].get_sockfd());

    //write end of names list to client
    std::string end_of_names_message =  ":server 366 "
                                        + this->_client[client_index].get_nick_name()
                                        + " "
                                        + tokenized_message.params[0]
                                        + " :End of /NAMES list\n";

    putstr_fd(end_of_names_message, this->_client[client_index].get_sockfd());
    //proper message and error handling still needed
}

void Server::commands_nick(struct msg_tokens tokenized_message, int client_index)
{
    int current_client_fd;

    for (unsigned int i = 0; i < this->_client.size(); i++)
    {
        if (tokenized_message.params[0] == this->_client[i].get_nick_name())
        {
            putstr_fd("nickname already in use sry bruh\n", this->_client[i].get_sockfd());
            return ;
        }
    }
    for (unsigned int i = 0; i < this->_client.size(); i++)
    {
        current_client_fd = this->_client[i].get_sockfd();
        putstr_fd(":", current_client_fd);
        putstr_fd(this->_client[client_index].get_nick_name(), current_client_fd);
        putstr_fd("!", current_client_fd);
        putstr_fd(this->_client[client_index].get_user_name(), current_client_fd);
        putstr_fd("@localhost NICK :", current_client_fd);
        putstr_fd(tokenized_message.params[0], current_client_fd);
        putstr_fd("\n", current_client_fd);
    }
    this->_client[client_index].set_nick_name(tokenized_message.params[0]);
}

void Server::commands_user(struct msg_tokens tokenized_message, int client_index)
{
    //check for already taken username still missing

    for (unsigned int i = 0; i < this->_client.size(); i++)
    {
        if (tokenized_message.params[0] == this->_client[i].get_user_name())
        {
            putstr_fd("username already in use sry bruh\n", this->_client[i].get_sockfd());
            return ;
        }
    }

    this->_client[client_index].set_user_name(tokenized_message.params[0]);

    putstr_fd(":server 001 ", this->_client[client_index].get_sockfd());
    putstr_fd(this->_client[client_index].get_nick_name(), this->_client[client_index].get_sockfd());
    putstr_fd(" :Welcome to the IRC Network ", this->_client[client_index].get_sockfd());
    putstr_fd(this->_client[client_index].get_nick_name(), this->_client[client_index].get_sockfd());
    putstr_fd("!", this->_client[client_index].get_sockfd());
    putstr_fd(this->_client[client_index].get_user_name(), this->_client[client_index].get_sockfd());
    putstr_fd("@localhost\n", this->_client[client_index].get_sockfd());
}
// https://datatracker.ietf.org/doc/html/rfc2812#section-5
// https://chi.cs.uchicago.edu/chirc/irc_examples.html
// different newer protocol also works for kvirc

void Server::commands_message(struct msg_tokens tokenized_message, int client_index)
{
    if (tokenized_message.params.empty() || tokenized_message.params[0].empty())
    {
        putstr_fd(":server 461 PRIVMSG :Not enough parameters\n", this->_client[client_index].get_sockfd());
        return ;
    }
    if (tokenized_message.params[0][0] == '#')
    {
    //channel
        int channel_index = this->get_channel_index_through_name(tokenized_message.params[0]);

        //if channel doesnt exist
        if (channel_index == -1)
            std::cout << "----------------channel doesnt exist" << std::endl;

        std::vector<std::string> client_list =  this->_channel[channel_index].get_client_list();
        for (unsigned int i = 0; i < client_list.size(); i++)
        {
            int current_client_index = this->get_client_index_through_name(client_list[i]);
            if (current_client_index != client_index)
            {
                std::string message =   ":"
                                        + this->_client[current_client_index].get_nick_name()
                                        + "!"
                                        + this->_client[current_client_index].get_user_name()
                                        + "@localhost PRIVMSG "
                                        + tokenized_message.params[0]
                                        + " :"
                                        + tokenized_message.trailing
                                        + "\n";
                putstr_fd(message, this->_client[current_client_index].get_sockfd());
            }
        }
    }
    else
    {
        int current_client_index = this->get_client_index_through_name(tokenized_message.params[0]);
        std::string message =   ":"
                                + this->_client[current_client_index].get_nick_name()
                                + "!"
                                + this->_client[current_client_index].get_user_name()
                                + "@localhost PRIVMSG "
                                + tokenized_message.params[0]
                                + " :"
                                + tokenized_message.trailing
                                + "\n";
        putstr_fd(message, this->_client[current_client_index].get_sockfd());
    }
}


void Server::execute_command(struct msg_tokens tokenized_message, int client_index)
{
    if (std::isdigit(tokenized_message.command[0]))
    {
        putstr_fd(":server ", this->_client[client_index].get_sockfd());
        putstr_fd(tokenized_message.command, this->_client[client_index].get_sockfd());
        putstr_fd(" ", this->_client[client_index].get_sockfd());
        putstr_fd(this->_client[client_index].get_nick_name(), this->_client[client_index].get_sockfd());
        putstr_fd(" :", this->_client[client_index].get_sockfd());
        putstr_fd(tokenized_message.params[0], this->_client[client_index].get_sockfd());
        putstr_fd("\n", this->_client[client_index].get_sockfd());
        return ;
    }
    if (!this->_client[client_index].get_authenticated() && tokenized_message.command != "PASS")
    {
        execute_command(error_message("464", "Password required"), client_index);
        return ;
    }
    if (tokenized_message.command == "PASS")
        authenticateClient(tokenized_message, client_index);
    else if (tokenized_message.command == "JOIN")
        this->commands_join(tokenized_message, client_index);
    else if (tokenized_message.command == "NICK")
        this->commands_nick(tokenized_message, client_index);
    else if (tokenized_message.command == "USER")
        this->commands_user(tokenized_message, client_index);
    else if (tokenized_message.command == "PRIVMSG")
        this->commands_message(tokenized_message, client_index);
    else if (tokenized_message.command == "PING")
    {
        putstr_fd(":server PONG ", this->_client[client_index].get_sockfd());
		if (tokenized_message.params.empty())
			putstr_fd("server", this->_client[client_index].get_sockfd());
		else
			putstr_fd(tokenized_message.params[0], this->_client[client_index].get_sockfd());
		putstr_fd("\n", this->_client[client_index].get_sockfd());
    }
    else
        execute_command(error_message("421", "Unknown command"), client_index);
}



void Server::handle_data(int client_index)
{
	std::stringstream       message_stream(this->_client[client_index].get_last_message());
	std::string             line;
	struct msg_tokens       tokenized_message;

	while (std::getline(message_stream, line))
	{
		if (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
			line.pop_back();
		if (!line.empty())
		{
			tokenized_message = this->parse_message_line(line);
            std::cout << this->_client[client_index].get_nick_name() << ": " << line << std::endl;
			this->execute_command(tokenized_message, client_index);
		}
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
		return (false);
	}
	else
	{
		std::cout << "set authenticated(true)" << std::endl;
		this->_client[client_index].set_authenticated(true);
		return (true);
	}
}
