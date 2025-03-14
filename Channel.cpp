
#include "Channel.hpp"

void    Channel::set_name(std::string new_name)
{
    this->_name = new_name;
}

std::string Channel::get_name()
{
    return(this->_name);
}

void Channel::add_client_to_list(std::string client_name)
{
    this->_client.emplace_back(client_name);
}

void Channel::remove_client_from_list(std::string client_name)
{
    for (unsigned int i = 0; i < this->_client.size(); i++)
    {
        if (this->_client[i] == client_name)
        {
            this->_client.erase(this->_client.begin() + i);
            break ;
        }
    }
}

std::vector<std::string> Channel::get_client_list()
{
    return (this->_client);
}