
#include "Server.hpp"
#include "../Channel/Channel.hpp"

Server::Server()
{
	try {
		_client.reserve(100);
		_channel.reserve(40);
	} catch (const std::bad_alloc &e)
	{
		std::cerr << "[ERROR] Memory allocation failed: " << e.what() << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

Server::~Server()
{
    std::cout << "Shutting down server..." << std::endl;

    for (auto &client : this->_client)
	{
		if (client.get_sockfd() != -1)
			close(client.get_sockfd());
	}
	this->_client.clear();
	this->_poll_fd.clear();
	this->_channel.clear();
	if (this->_sockfd != -1)
		close(this->_sockfd);
}

Server::msg_tokens Server::error_message(std::string error_code, std::string message)
{
    msg_tokens error_message;
    error_message.command = error_code;
    error_message.params.push_back(message);
    return (error_message);
}

void Server::send_error_message(int client_index, std::string error_code, std::string message)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
    msg_tokens error_msg = error_message(error_code, message);
    std::string formatted_error = ":" + error_msg.command + " " + error_msg.params[0] + "\n";
    putstr_fd(formatted_error, this->_client[client_index].get_sockfd());
}

bool Server::create_socket()
{
    this->_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_sockfd < 0)
	{
		std::cerr << "[ERROR] Failed to create socket: " << strerror(errno) << std::endl;
		return (false); // Instead of throwing an exception, return false
	}
	return (true);
}

bool Server::fill_socket_struct()
{
    int enabled = 1;

    bzero((char *) &this->_server_address, sizeof(this->_server_address));
    this->_server_address.sin_family = AF_INET;
    this->_server_address.sin_addr.s_addr = INADDR_ANY;
    this->_server_address.sin_port = htons(this->_port);
	if (setsockopt(this->_sockfd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)) < 0)
	{
		std::cerr << "[ERROR] setsockopt failed: " << strerror(errno) << std::endl;
		close(this->_sockfd);
		return (false);
	}
	if (fcntl(this->_sockfd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "[ERROR] fcntl failed: " << strerror(errno) << std::endl;
		return (false);
	}
	return (true);
}

bool Server::bind_server_address()
{
    if (bind(this->_sockfd, (struct sockaddr *) &this->_server_address, sizeof(this->_server_address)) < 0)
    {
        std::cerr << "[ERROR] Failed to bind socket: " << strerror(errno) << std::endl;
        return (false);
    }
    return (true);
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

bool Server::init(char **av)
{
	this->_port = std::atoi(av[1]);
	this->_password = av[2];
	if (this->_port < 1024 || this->_port > 65535)
	{
		std::cerr << "[ERROR] Invalid port number: " << this->_port << ". Port must be between 1024 and 65535.\n";
		return (false);
	}
	if (!this->create_socket() || !this->fill_socket_struct() || !this->bind_server_address())
	{
		close(this->_sockfd);
		return (false);
	}
	if (listen(this->_sockfd, 5) < 0)
	{
		std::cerr << "[ERROR] Failed to listen on socket: " << strerror(errno) << std::endl;
		close(this->_sockfd);
		return (false);
	}
	this->init_poll_struct(this->_sockfd);
	this->running = true;
	std::cout << "[INFO] Server initialized successfully on port " << this->_port << std::endl;
	return (true);
}

void Server::accept_client()
{
    struct sockaddr_in   temp_client_address;
    socklen_t            temp_client_len;

	int new_sockfd = accept(this->_sockfd, (struct sockaddr *)&temp_client_address, &temp_client_len);
	if (new_sockfd < 0)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return;
		std::cerr << "[ERROR] Failed to accept client: " << strerror(errno) << std::endl;
		return ;
	}

	try
	{
		Client new_client;
		new_client.set_sockfd(new_sockfd);
		new_client.set_address(temp_client_address);
		new_client.set_len(temp_client_len);
		new_client.set_nick_name("none");
		std::cout << "[INFO] New client connected (fd: " << new_sockfd << ")" << std::endl;
		this->_client.emplace_back(new_client);
		this->init_poll_struct(new_client.get_sockfd());
	}
	catch (const std::bad_alloc &e)
	{
		std::cerr << "[ERROR] Memory allocation failed while adding client: " << e.what() << std::endl;
		close(new_sockfd);
	}
}

void Server::disconnect_client(int client_index)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
    std::string nick = this->_client[client_index].get_nick_name();
    std::cout << "Client " << nick << " disconnected." << std::endl;

    int client_fd = this->_client[client_index].get_sockfd();
	_poll_fd.erase(
        std::remove_if(_poll_fd.begin(), _poll_fd.end(),
                       [client_fd](const struct pollfd &p)
					   { return p.fd == client_fd; }),
        _poll_fd.end());
	close(client_fd);
	_client.erase(_client.begin() + client_index);
    for (auto &channel : _channel)
        channel.remove_client_from_list(nick);
}

//not a fatal error should not quit whole server
// void Server::receive_data(int client_index)
// {
//     char    buffer[1028];
//     int     n = read(this->_client[client_index].get_sockfd(), buffer, sizeof(buffer) - 1);

//     if (n < 0)
//         throw std::invalid_argument ("ERROR reading from socket");
//     this->_client[client_index].set_last_message(buffer);
// } // debug

void Server::receive_data(int client_index)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return ;

	char buffer[1024];
	int client_fd = _client[client_index].get_sockfd();
	int n = read(client_fd, buffer, sizeof(buffer) - 1);

	if (n < 0)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return ;
		std::cerr << "[ERROR] Read error from client " << client_fd << ": " << strerror(errno) << std::endl;
		send_error_message(client_index, "ERROR", strerror(errno));
		disconnect_client(client_index);
		return ;
	}
	if (n == 0)
	{
		disconnect_client(client_index);
		return ;
	}
	buffer[n] = '\0';
	std::string_view last_message = _client[client_index].get_last_message();
	if (last_message.size() + n > 1024)
	{
		std::cerr << "[WARNING] Client " << client_fd
				<< " exceeded max message size. Clearing buffer.\n";
		_client[client_index].set_last_message("");
		send_error_message(client_index, "417", "Message too long. Limit is 512 bytes.");
		return ;
	}
	try
	{
		_client[client_index].append_last_message(buffer);
	}
	catch (const std::exception &e)
	{
		std::cerr << "[ERROR] Failed to append message: " << e.what() << std::endl;
	}
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
	while (this->running)
	{
		int ret = poll(&_poll_fd[0], _poll_fd.size(), 10000);
		if (ret == -1)
		{
			if (errno == EINTR)
				continue ;
			std::cerr << "[ERROR] Poll failure: " << strerror(errno) << std::endl;
						if (_poll_fd.empty()) {
				std::cerr << "[CRITICAL] No valid sockets remaining. Exiting...\n";
				break ;
			}
			continue ;
		}
		for (int i = _poll_fd.size() - 1; i >= 0; i--)
		{
			if (_poll_fd[i].revents & POLLIN)
			{
				if (i == 0)
					accept_client();
				else
				{
					int client_index = i - 1;
					if (client_index >= 0 && client_index < static_cast<int>(_client.size()))
					{
						receive_data(client_index);
						if (client_index < static_cast<int>(_client.size()) && !_client[client_index].get_last_message().empty() && _client[client_index].get_last_message().back() == '\n')
							handle_data(client_index);
					}
				}
			}
		}
		cleanup_disconnected_clients();
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
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return (false);
	std::string password;
	if (!tokenized_message.params.empty())
		password = tokenized_message.params[0];
	else
		password = tokenized_message.trailing;
	//std::cout << "Print password:" << password << std::endl;
	//std::cout << "Print server password:" << this->_password << std::endl;
	if (password != this->_password)
	{
		this->_client[client_index].increase_failed_auth_attempts();
		if (this->_client[client_index].get_failed_auth_attempts() >= 3)
		{
			std::string error_msg = ":server 464 " + this->_client[client_index].get_nick_name() +
									" :Too many failed authentication attempts. Disconnecting...\n";
			putstr_fd(error_msg, this->_client[client_index].get_sockfd());
			std::string quit_msg = ":" + this->_client[client_index].get_nick_name() +
								" QUIT :Excessive failed authentication\n";
			putstr_fd(quit_msg, this->_client[client_index].get_sockfd());
			disconnect_client(client_index);
			return (false);
		}
		std::string passwd_mismatch_msg = ":server 464 " + this->_client[client_index].get_nick_name() +
										" :Incorrect password. Try again.\n";
		putstr_fd(passwd_mismatch_msg, this->_client[client_index].get_sockfd());
		return (false);
	}
	this->_client[client_index].reset_failed_auth_attempts();
	this->_client[client_index].set_authenticated(true);
	return (true);
}
