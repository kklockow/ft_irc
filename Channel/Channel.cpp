
#include "Channel.hpp"

void    Channel::set_name(std::string new_name)
{
    this->_name = new_name;
}

std::string Channel::get_name() const
{
    return(this->_name);
}

void Channel::add_client_to_list(const std::string &client_name)
{
	if(std::find(this->_client.begin(), this->_client.end(), client_name) == this->_client.end())
    	this->_client.emplace_back(client_name);
}

void Channel::remove_client_from_list(const std::string &client_name)
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

const std::vector<std::string>& Channel::get_client_list() const
{
    return this->_client;
}

void Channel::add_operator_to_channel(const std::string &client_name)
{
	if(std::find(this->_operators.begin(), this->_operators.end(), client_name) == this->_operators.end())
		this->_operators.emplace_back(client_name);
}

void Channel::remove_operator_from_channel(const std::string &client_name)
{
	for (unsigned int i = 0; i < this->_operators.size(); i++)
    {
        if (this->_operators[i] == client_name)
        {
            this->_operators.erase(this->_operators.begin() + i);
            break ;
        }
    }
}

const std::vector<std::string>& Channel::get_operator_list() const
{
	return (this->_operators);
}

void Channel::add_invited_to_channel(const std::string &client_name)
{
	if (std::find(this->_invited.begin(), this->_invited.end(), client_name) == this->_invited.end())
		this->_invited.emplace_back(client_name);
}


void Channel::remove_invited_from_channel(const std::string &client_name)
{
	for (unsigned int i = 0; i < this->_invited.size(); i++)
    {
        if (this->_invited[i] == client_name)
        {
            this->_invited.erase(this->_invited.begin() + i);
            break ;
        }
    }
}

const std::vector<std::string>& Channel::get_invited_list() const
{
	return (this->_invited);
}

bool Channel::is_client_in_list(const std::string &client_name, const std::vector<std::string> &list) const
{
	return std::find(list.begin(), list.end(), client_name) != list.end();
}