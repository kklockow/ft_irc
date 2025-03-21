
#include "Server.hpp"

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

bool Server::valid_channel_index(int index)
{
	if (index < 0 || index >= static_cast<int>(_channel.size()))
	{
		std::cerr << "[ERROR] Invalid channel index: " << index << std::endl;
		return (false);
	}
	return (true);
}

bool Server::valid_client_index(int index)
{
	if (index < 0 || index >= static_cast<int>(_client.size()))
	{
		std::cerr << "[ERROR] Invalid client index: " << index << std::endl;
		return (false);
	}
	return (true);
}

/// @brief Removes disconnected clients from the client vector
/// @details Iterates through the client vector and removes any clients that have a socket file descriptor of -1
/// @note It uses a lambda function to remove the disconnected clients
void Server::cleanup_disconnected_clients()
{
    _client.erase(std::remove_if(_client.begin(), _client.end(),
                                 [](const Client &c) { return c.get_sockfd() == -1; }),
                  _client.end());
}

Client* Server::get_client_by_nickname(const std::string &nickname)
{
	int index = get_client_index_through_name(nickname);
	if (index == -1)
		return (NULL);
	return &(_client[index]);
}

Channel* Server::get_channel_by_name(const std::string &channel_name)
{
	int index = get_channel_index_through_name(channel_name);
	if (index < 0)
		return nullptr;
	return &(_channel[index]);
}

Client* Server::get_client_by_fd(int fd)
{
    for (std::vector<Client>::iterator it = _client.begin(); it != _client.end(); ++it) {
        if (it->get_sockfd() == fd)
            return &(*it);
    }
    return nullptr;
}

void Server::send_to_client(const std::string &msg, Client &client) {
    // Append the message to the client's output buffer.
    client.output_buffer.append(msg);
    // Find the client's pollfd entry and enable POLLOUT.
    for (size_t i = 0; i < _poll_fd.size(); ++i) 
	{
        if (_poll_fd[i].fd == client.get_sockfd()) 
		{
            _poll_fd[i].events |= POLLOUT;
            break;
        }
    }
}
