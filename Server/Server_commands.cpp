
#include "Server.hpp"

// void Server::execute_command(struct msg_tokens tokenized_message, int client_index)
// {
// 	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
// 		return;
//     if (std::isdigit(tokenized_message.command[0]))
//     {
//         send_to_client(":server ", this->_client[client_index].get_sockfd());
//         send_to_client(tokenized_message.command, this->_client[client_index].get_sockfd());
//         send_to_client(" ", this->_client[client_index].get_sockfd());
//         send_to_client(this->_client[client_index].get_nick_name(), this->_client[client_index].get_sockfd());
//         send_to_client(" :", this->_client[client_index].get_sockfd());
//         send_to_client(tokenized_message.params[0], this->_client[client_index].get_sockfd());
//         send_to_client("\n", this->_client[client_index].get_sockfd());
//     }
//     else if (!this->_client[client_index].get_authenticated() && tokenized_message.command != "PASS")
//         execute_command(error_message("464", "Password required"), client_index);
//     else if (tokenized_message.command == "PASS")
//         authenticateClient(tokenized_message, client_index);
//     else if (tokenized_message.command == "JOIN")
//         this->commands_join(tokenized_message, client_index);
//     else if (tokenized_message.command == "NICK")
//         this->commands_nick(tokenized_message, client_index);
//     else if (tokenized_message.command == "USER")
//         this->commands_user(tokenized_message, client_index);
//     else if (tokenized_message.command == "PRIVMSG")
//         this->commands_message(tokenized_message, client_index);
//     else if (tokenized_message.command == "PART")
//         this->commands_part(tokenized_message, client_index);
//     else if (tokenized_message.command == "PING")
//         this->commands_ping(tokenized_message, client_index);
// 	else if (tokenized_message.command == "QUIT")
// 		this->commands_quit(tokenized_message, client_index);
// 	else if (tokenized_message.command == "KICK" || tokenized_message.command == "INVITE" ||
// 				tokenized_message.command == "TOPIC" || tokenized_message.command == "MODE")
// 	{
// 		int channel_index = this->get_channel_index_through_name(tokenized_message.params[0]);
// 		OP_commands::execute_operator_cmd(tokenized_message, this->_client[client_index].get_nick_name(), this->_channel[channel_index]);
// 	}
//     else
//         execute_command(error_message("421", "Unknown command"), client_index);

//     // reset message after command
//     this->_client[client_index].set_last_message((char *)"");
// }

void Server::execute_command(Server::msg_tokens tokenized_message, int client_index)
{
    if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
	auto it = command_map.find(tokenized_message.command);
	if (std::isdigit(tokenized_message.command[0]))
		put_str_fd(tokenized_message, client_index);
	else if (!this->_client[client_index].get_authenticated() && tokenized_message.command != "PASS")
		execute_command(error_message("464", "Password required"), client_index);
	else if (tokenized_message.command == "PASS")
		authenticateClient(tokenized_message, client_index);
	else if (tokenized_message.command == "KICK" || tokenized_message.command == "INVITE" ||
			tokenized_message.command == "TOPIC" || tokenized_message.command == "MODE")
	{
		execute_operator_cmd(tokenized_message, this->_client[client_index], *this);
	}
    else if (it != command_map.end() && it->second.handler)
        (this->*(it->second.handler))(tokenized_message, client_index);
    else
        execute_command(error_message("421", "Unknown command"), client_index);
    this->_client[client_index].set_last_message((char *)"");
}

void Server::put_str_fd(Server::msg_tokens tokenized_message, int client_index)
{
	send_to_client(":server ", this->_client[client_index]);
	send_to_client(tokenized_message.command, this->_client[client_index]);
	send_to_client(" ", this->_client[client_index]);
	send_to_client(this->_client[client_index].get_nick_name(), this->_client[client_index]);
	send_to_client(" :", this->_client[client_index]);
	send_to_client(tokenized_message.params[0], this->_client[client_index]);
	send_to_client("\n", this->_client[client_index]);
}

void Server::commands_ping(struct msg_tokens tokenized_message, int client_index)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
    send_to_client(":server PONG ", this->_client[client_index]);
    if (tokenized_message.params.empty())
        send_to_client("server", this->_client[client_index]);
    else
        send_to_client(tokenized_message.params[0], this->_client[client_index]);
    send_to_client("\n", this->_client[client_index]);
}

void Server::commands_join_message_clients(std::string channel_name, int client_index)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
    int                         channel_index = 0;
    std::vector<std::string>    client_list;
	int							current_client_index = 0;

	if (!valid_client_index(client_index))
    {
        std::cerr << "[ERROR] Invalid client index: " << client_index << std::endl;
        return;
    }
    channel_index = this->get_channel_index_through_name(channel_name);
	if (!valid_channel_index(channel_index))
	{
		std::cerr << "[ERROR] Channel " << channel_name << " does not exist." << std::endl;
		return ;
	}
    client_list = this->_channel[channel_index].get_client_list();
    for (unsigned int i = 0; i < client_list.size(); i++)
    {
		current_client_index = get_client_index_through_name(client_list[i]);
		std::string message_to_clients =    ":"
										+ this->_client[client_index].get_nick_name()
										+ "!"
										+ this->_client[client_index].get_user_name()
										+ "@localhost JOIN "
										+ channel_name
										+ "\n";
		send_to_client(message_to_clients, this->_client[current_client_index]);
    }
}

void Server::commands_join(struct msg_tokens tokenized_message, int client_index)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
    Channel                     new_channel;
    int                         channel_index;
    std::vector<std::string>    client_list;


    if (tokenized_message.params.empty() || tokenized_message.params[0].empty())
    {
        send_to_client(":server 461 JOIN :Not enough parameters\n", this->_client[client_index]);
        return ;
    }

    channel_index = this->get_channel_index_through_name(tokenized_message.params[0]);
    //if channel does exist
    if (channel_index != -1)
    {
		if (!valid_channel_index(channel_index))
			return ;
        client_list = this->_channel[channel_index].get_client_list();
		if (this->_channel[channel_index].invite_only)
		{
			if(!this->_channel[channel_index].is_client_in_list(this->_client[client_index].get_nick_name(), this->_channel[channel_index].get_invited_list()))
            {
                std::string err_Msg = ":server 473 "
                                    + this->_client[client_index].get_nick_name() + " "
                                    + this->_channel[channel_index].get_name()
                                    + " :Cannot join channel (invite only)\r\n";
                send_to_client(err_Msg, this->_client[client_index]);
                return;
            }
		}
		if (this->_channel[channel_index].user_limit <= client_list.size())
		{
			std::string err_Msg = ":server 471 "
								+ this->_client[client_index].get_nick_name() + " "
								+ this->_channel[channel_index].get_name()
								+ " :Cannot join channel (Channel is full)\r\n";
			send_to_client(err_Msg, this->_client[client_index]);
			return;
		}
		if (!this->_channel[channel_index].get_password().empty()
			&& !this->_channel[channel_index].is_client_in_list(this->_client[client_index].get_nick_name(), this->_channel[channel_index].get_invited_list()))
		{
			if (tokenized_message.params.size() < 2)
			{
				send_to_client(":server 461 JOIN :Not enough parameters - channel key required\n", this->_client[client_index]);
				return;
			}
			else if (tokenized_message.params[1] != this->_channel[channel_index].get_password())
			{
				send_to_client(":server 475 " + tokenized_message.params[0] + " :Cannot join channel (Incorrect channel key)\n", this->_client[client_index]);
				return;
			}
		}
        for (unsigned int i = 0; i < client_list.size(); i++)
        {
            if (client_list[i] == this->_client[client_index].get_nick_name())
            {
                send_to_client(":server 443 " + this->_client[client_index].get_nick_name() + " "
                        + tokenized_message.params[0] + " :You're already in this channel\n",
                        this->_client[client_index]);
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
		//add first client to operators list of new channel
		new_channel.add_operator_to_channel(this->_client[client_index].get_nick_name());
        //add channel to channel vector
        this->_channel.emplace_back(new_channel);
        channel_index = this->_channel.size() - 1;
    }
    this->commands_join_message_clients(tokenized_message.params[0], client_index);
    //handle Channel desription
	if (this->_channel[channel_index].get_topic().empty())
	{
		std::string reply = ":server 331 " 
							+ this->_client[client_index].get_nick_name() + " "
							+ tokenized_message.params[0]
							+ " :No topic is set\r\n";
		send_to_client(reply, this->_client[client_index]);
	}
	else
	{
		std::string reply = ":server 332 " 
							+ this->_client[client_index].get_nick_name() + " "
							+ tokenized_message.params[0]
							+ " :" + this->_channel[channel_index].get_topic() + "\r\n";
		send_to_client(reply, this->_client[client_index]);
	}
    // write a list of all clients in channel to new joined client
    std::string client_list_message =   ":server 353 "
                                        + this->_client[client_index].get_nick_name()
                                        + " = "
                                        + tokenized_message.params[0]
                                        + " :";

    client_list = this->_channel[channel_index].get_client_list();
    for (unsigned int i = 0; i < client_list.size(); i++)
    {
		std::string name = client_list[i];
		if (this->_channel[channel_index].is_client_in_list(name, this->_channel[channel_index].get_operator_list()))
			name = "@"+ name;
        client_list_message += name + " ";
	}
    client_list_message += "\n";

    send_to_client(client_list_message, this->_client[client_index]);

    //write end of names list to client
    std::string end_of_names_message =  ":server 366 "
                                        + this->_client[client_index].get_nick_name()
                                        + " "
                                        + tokenized_message.params[0]
                                        + " :End of /NAMES list\n";

    send_to_client(end_of_names_message, this->_client[client_index]);
	//removes client from invited list
	this->_channel[channel_index].remove_invited_from_channel(this->_client[client_index].get_nick_name());
    //proper message and error handling still needed
}

void Server::commands_nick(struct msg_tokens tokenized_message, int client_index)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;

	if (tokenized_message.params.empty() || tokenized_message.params[0].empty())
	{
		send_to_client(":server 461 NICK :Not enough parameters\n", this->_client[client_index]);
		return;
	}
	std::string requested_nick = tokenized_message.params[0];
	std::string new_nick = requested_nick;
	int suffix = 1;
	while (true)
	{
		bool name_taken = false;
		for (unsigned int i = 0; i < this->_client.size(); i++)
		{
			if (this->_client[i].get_nick_name() == new_nick)
			{
				name_taken = true;
				break ;
			}
		}
		if (!name_taken)
			break ;
		new_nick = requested_nick + std::to_string(suffix);
		suffix++;
	}
	std::string server_answer = ":" 
								+ this->_client[client_index].get_nick_name()
								+ "!"
								+ this->_client[client_index].get_user_name()
								+ "@localhost NICK :"
								+ new_nick
								+ "\n";
	for (unsigned int i = 0; i < this->_client.size(); i++)
	{
		send_to_client(server_answer, this->_client[i]);
	}
	this->_client[client_index].set_nick_name(new_nick);
	if (new_nick != requested_nick)
	{
		std::string nickname_changed_message = ":server 433 " + requested_nick 
											+ " :Nickname is already in use. Assigned new nickname: " + new_nick + "\n";
		send_to_client(nickname_changed_message, this->_client[client_index]);
	}
}

void Server::commands_user(struct msg_tokens tokenized_message, int client_index)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
    //check for already taken username
    // for (unsigned int i = 0; i < this->_client.size(); i++)
    // {
    //     if (tokenized_message.params[0] == this->_client[i].get_user_name())
    //     {
    //         std::string already_in_use_message =    ":server 466 <username> :"
    //                                                 + tokenized_message.params[0]
    //                                                 + " is already in use\n";

    //         send_to_client(already_in_use_message, this->_client[i].get_sockfd());
    //         return ;
    //     }
    // }

    this->_client[client_index].set_user_name(tokenized_message.params[0]);

    std::string welcome_message =   ":server 001 "
                                    + this->_client[client_index].get_nick_name()
                                    + " :Welcome to the IRC Network "
                                    + this->_client[client_index].get_nick_name()
                                    + "!"
                                    + this->_client[client_index].get_user_name()
                                    + "@localhost\n";

    send_to_client(welcome_message, this->_client[client_index]);
}
// https://datatracker.ietf.org/doc/html/rfc2812#section-5
// https://chi.cs.uchicago.edu/chirc/irc_examples.html
// different newer protocol also works for kvirc

void Server::commands_message(struct msg_tokens tokenized_message, int client_index)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
    if (tokenized_message.params.empty() || tokenized_message.params[0].empty())
    {
        send_to_client(":server 461 PRIVMSG :Not enough parameters\n", this->_client[client_index]);
        return;
    }
    std::string target = tokenized_message.params[0];
    if (target[0] == '#')
    {
        int channel_index = this->get_channel_index_through_name(target);
        if (!valid_channel_index(channel_index))
        {
            std::string server_message = ":server 403 " + target + " :No such channel\n";
            send_to_client(server_message, this->_client[client_index]);
            return;
        }
        std::vector<std::string> client_list = this->_channel[channel_index].get_client_list();
        for (const std::string &client_name : client_list)
        {
            int recipient_index = this->get_client_index_through_name(client_name);
            if (valid_client_index(recipient_index) && recipient_index != client_index)
            {
                std::string message = ":"
                                      + this->_client[client_index].get_nick_name()
                                      + "!"
                                      + this->_client[client_index].get_user_name()
                                      + "@localhost PRIVMSG "
                                      + target
                                      + " :"
                                      + tokenized_message.trailing
                                      + "\n";
                send_to_client(message, this->_client[recipient_index]);
            }
        }
    }
    else
    {
        int recipient_index = this->get_client_index_through_name(target);
        if (!valid_client_index(recipient_index))
        {
            std::string server_message = ":server 401 " + target + " :No such nick\n";
            send_to_client(server_message, this->_client[client_index]);
            return;
        }
        std::string message = ":"
                              + this->_client[client_index].get_nick_name()
                              + "!"
                              + this->_client[client_index].get_user_name()
                              + "@localhost PRIVMSG "
                              + target
                              + " :"
                              + tokenized_message.trailing
                              + "\n";
        send_to_client(message, this->_client[recipient_index]);
    }
}

void Server::commands_part(struct msg_tokens tokenized_message, int client_index)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
    //check if params vector is empty
    if (tokenized_message.params.empty() || tokenized_message.params[0].empty())
    {
        send_to_client(":server 461 PART :Not enough parameters\n", this->_client[client_index]);
        return ;
    }

    int channel_index = this->get_channel_index_through_name(tokenized_message.params[0]);

    // channel doesnt exist
    if (!(valid_channel_index(channel_index)))
    {
        std::string nonexist_message = ":server 403 " + tokenized_message.params[0] + " :No such channel\n";
        send_to_client(nonexist_message, this->_client[client_index]);
        return ;
    }

    std::vector<std::string> client_list = this->_channel[channel_index].get_client_list();
    for (unsigned int i = 0; i < client_list.size(); i++)
    {
        if (this->_client[client_index].get_nick_name() == client_list[i])
        {
            // boradcast user leaving channel
            for (unsigned int i = 0; i < client_list.size(); i++)
            {
                int current_client_index = this->get_client_index_through_name(client_list[i]);
                std::string message =   ":"
                                        + this->_client[client_index].get_nick_name()
                                        + "!"
                                        + this->_client[client_index].get_user_name()
                                        + "@localhost PART "
                                        + tokenized_message.params[0]
                                        + "\n";
                send_to_client(message, this->_client[current_client_index]);
            }
            this->_channel[channel_index].remove_client_from_all_lists(this->_client[client_index].get_nick_name());
			if (_channel[channel_index].get_operator_list().empty() && !_channel[channel_index].get_client_list().empty())
			{
				std::string notify_new_operator = ":server MODE "
													+ _channel[channel_index].get_name() + " +o "
													+ _channel[channel_index].get_client_list().front() + "\r\n";
				_channel[channel_index].add_operator_to_channel(_channel[channel_index].get_client_list().front());	
				for (std::vector<std::string>::const_iterator it = _channel[channel_index].get_client_list().begin(); it != _channel[channel_index].get_client_list().end(); ++it)
				{
					Client* client = get_client_by_nickname(*it);
					if (client)
					send_to_client(notify_new_operator, *client);
				}
			}
			
				cleanup_empty_channels();
            return ;
        }
    }
    // user not on channel
    std::string not_in_channel_message = ":server 442 " + tokenized_message.params[0] + " :You're not on that channel\n";
    send_to_client(not_in_channel_message, this->_client[client_index]);
}

void Server::commands_quit(struct msg_tokens tokenized_message, int client_index)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
    std::string nick = this->_client[client_index].get_nick_name();
    std::string quit_message = "Client Disconnected";

    if (!tokenized_message.trailing.empty())
        quit_message = tokenized_message.trailing;

    std::string quit_msg = ":" + nick + " QUIT :" + quit_message + "\n";
    for (auto &channel : this->_channel)
    {
		int channel_index = get_channel_index_through_name(channel.get_name());
		if (!valid_channel_index(channel_index))
			continue ;
        if (std::find(channel.get_client_list().begin(), channel.get_client_list().end(), nick) != channel.get_client_list().end())
        {
            for (const std::string &client_name : channel.get_client_list())
            {
                int target_index = get_client_index_through_name(client_name);
                if (target_index != -1 && target_index != client_index)
                    send_to_client(quit_msg, this->_client[target_index]);
            }
            channel.remove_client_from_all_lists(nick);
			if (channel.get_operator_list().empty() && !channel.get_client_list().empty())
			{
				std::string notify_new_operator = ":server MODE "
												+ channel.get_name() + " +o "
												+ channel.get_client_list().front() + "\r\n";
				channel.add_operator_to_channel(channel.get_client_list().front());
				for (std::vector<std::string>::const_iterator it = channel.get_client_list().begin(); it != channel.get_client_list().end(); ++it)
				{
					Client* client = get_client_by_nickname(*it);
					if (client)
					send_to_client(notify_new_operator, *client);
				}
			}
        }
    }
    send_to_client(quit_msg, this->_client[client_index]);	
	cleanup_empty_channels();
    disconnect_client(client_index);
}
