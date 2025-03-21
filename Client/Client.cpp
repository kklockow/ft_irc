
#include "Client.hpp"

Client::Client() : _failed_auth_attempts(0) {}

void Client::set_sockfd(int new_sockfd)
{
    this->_sockfd = new_sockfd;
}

int Client::get_sockfd() const
{
    return (this->_sockfd);
}

void Client::set_address(struct sockaddr_in new_sockaddr_in)
{
    this->_client_address = new_sockaddr_in;
}

struct sockaddr_in Client::get_address()
{
    return (this->_client_address);
}

void Client::set_len(socklen_t new_client_len)
{
    this->_client_len = new_client_len;
}

socklen_t Client::get_len()
{
    return (this->_client_len);
}

void Client::set_nick_name(std::string new_nickname)
{
    this->_nick_name = new_nickname;
}

std::string Client::get_nick_name()
{
    return (this->_nick_name);
}

void Client::set_last_message(const std::string& message)
{
    std::string temp_message(message);

    this->_last_message = temp_message;
}

void Client::append_last_message(char *message)
{
    std::string temp_message(message);

    this->_last_message += temp_message;
}

std::string Client::get_last_message()
{
    return (this->_last_message);
}

void Client::set_authenticated(bool flag)
{
	this->_authenticated = flag;
}

bool Client::get_authenticated()
{
	return (this->_authenticated);
}

void Client::set_user_name(std::string new_username)
{
	this->_user_name = new_username;
}

std::string Client::get_user_name()
{
	return (this->_user_name);
}

void Client::increase_failed_auth_attempts()
{
	_failed_auth_attempts++;
}

void  Client::reset_failed_auth_attempts()
{
	_failed_auth_attempts = 0;
}

int  Client::get_failed_auth_attempts() const
{
	return (_failed_auth_attempts);
}
