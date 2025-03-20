
#pragma once

#include <iostream>
#include <vector>
#include <algorithm>

class Channel
{
	private:
		std::string					_password;
		std::string					_topic;
		std::string                 _name;
		std::vector<std::string>    _client;
		std::vector<std::string>	_operators;
		std::vector<std::string>	_invited;

	public:
		unsigned int						user_limit = 1024;
		bool								topic_restriction = false;
		bool								invite_only = false;
		void                              	set_name(std::string new_name);
		std::string                       	get_name() const;
		void                              	add_client_to_list(const std::string &client_name);
		void                              	remove_client_from_list(const std::string &client_name);
		const std::vector<std::string>&   	get_client_list() const;
		void								add_operator_to_channel(const std::string &client_name);
		void								remove_operator_from_channel(const std::string &client_name);
		const std::vector<std::string>&		get_operator_list() const;
		std::string							get_topic() const { return _topic; };
		void								set_topic(const std::string& new_topic) { _topic = new_topic; };
		std::string							get_password() const { return _password; };
		void								set_password(const std::string& new_password) {_password = new_password; };
		void								remove_invited_from_channel(const std::string &client_name);
		void								add_invited_to_channel(const std::string &client_name);
		const std::vector<std::string>&		get_invited_list() const;
		bool								is_client_in_list(const std::string &client_name, const std::vector<std::string> &list) const;
		void								remove_client_from_all_lists(const std::string &client_name);
};
