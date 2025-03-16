
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
