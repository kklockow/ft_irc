
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
    for (auto &it : this->_client)
    {
        if (it == client_name)
            it.erase();
    }
}