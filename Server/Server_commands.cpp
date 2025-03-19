
#include "Server.hpp"

// void Server::execute_command(struct msg_tokens tokenized_message, int client_index)
// {
// 	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
// 		return;
//     if (std::isdigit(tokenized_message.command[0]))
//     {
//         putstr_fd(":server ", this->_client[client_index].get_sockfd());
//         putstr_fd(tokenized_message.command, this->_client[client_index].get_sockfd());
//         putstr_fd(" ", this->_client[client_index].get_sockfd());
//         putstr_fd(this->_client[client_index].get_nick_name(), this->_client[client_index].get_sockfd());
//         putstr_fd(" :", this->_client[client_index].get_sockfd());
//         putstr_fd(tokenized_message.params[0], this->_client[client_index].get_sockfd());
//         putstr_fd("\n", this->_client[client_index].get_sockfd());
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
	putstr_fd(":server ", this->_client[client_index].get_sockfd());
	putstr_fd(tokenized_message.command, this->_client[client_index].get_sockfd());
	putstr_fd(" ", this->_client[client_index].get_sockfd());
	putstr_fd(this->_client[client_index].get_nick_name(), this->_client[client_index].get_sockfd());
	putstr_fd(" :", this->_client[client_index].get_sockfd());
	putstr_fd(tokenized_message.params[0], this->_client[client_index].get_sockfd());
	putstr_fd("\n", this->_client[client_index].get_sockfd());
}

void Server::commands_ping(struct msg_tokens tokenized_message, int client_index)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
    putstr_fd(":server PONG ", this->_client[client_index].get_sockfd());
    if (tokenized_message.params.empty())
        putstr_fd("server", this->_client[client_index].get_sockfd());
    else
        putstr_fd(tokenized_message.params[0], this->_client[client_index].get_sockfd());
    putstr_fd("\n", this->_client[client_index].get_sockfd());
}

void Server::commands_join_message_clients(std::string channel_name, int client_index)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
    int                         channel_index = 0;
    std::vector<std::string>    client_list;

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
		std::string message_to_clients =    ":"
										+ this->_client[client_index].get_nick_name()
										+ "!"
										+ this->_client[client_index].get_user_name()
										+ "@localhost JOIN "
										+ channel_name
										+ "\n";
		putstr_fd(message_to_clients, this->_client[client_index ].get_sockfd());
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
        putstr_fd(":server 461 JOIN :Not enough parameters\n", this->_client[client_index].get_sockfd());
        return ;
    }

    channel_index = this->get_channel_index_through_name(tokenized_message.params[0]);
    //if channel does exist
    if (channel_index != -1)
    {
		if (valid_channel_index(channel_index))
			return ;
        client_list = this->_channel[channel_index].get_client_list();
		if (this->_channel[channel_index].invite_only)
		{
			std::string err_Msg = ":server 473 "
								+ this->_client[client_index].get_nick_name() + " "
								+ this->_channel[channel_index].get_name()
								+ " :Cannot join channel (invite only)\r\n";
			putstr_fd(err_Msg, this->_client[client_index].get_sockfd());
			return;
		}
		if (this->_channel[channel_index].user_limit <= client_list.size())
		{
			std::string err_Msg = ":server 471 "
								+ this->_client[client_index].get_nick_name() + " "
								+ this->_channel[channel_index].get_name()
								+ " :Cannot join channel (Channel is full)\r\n";
			putstr_fd(err_Msg, this->_client[client_index].get_sockfd());
			return;
		}
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
		putstr_fd(reply, this->_client[client_index].get_sockfd());
	}
	else
	{
		std::string reply = ":server 332 " 
							+ this->_client[client_index].get_nick_name() + " "
							+ tokenized_message.params[0]
							+ " :" + this->_channel[channel_index].get_topic() + "\r\n";
		putstr_fd(reply, this->_client[client_index].get_sockfd());
	}
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
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
    int current_client_fd;


    if (tokenized_message.params.empty() || tokenized_message.params[0].empty())
    {
        putstr_fd(":server 461 JOIN :Not enough parameters\n", this->_client[client_index].get_sockfd());
        return ;
    }

    // search for nickname in client vector
    for (unsigned int i = 0; i < this->_client.size(); i++)
    {
        if (tokenized_message.params[0] == this->_client[i].get_nick_name())
        {
            std::string already_in_use_message =    "433 <nickname> :"
                                                    + tokenized_message.params[0]
                                                    + " is already in use\n";

            putstr_fd(already_in_use_message, this->_client[client_index].get_sockfd());
            return ;
        }
    }
    // broadcast that nick was changed
    for (unsigned int i = 0; i < this->_client.size(); i++)
    {
        current_client_fd = this->_client[i].get_sockfd();
        std::string server_answer = ":"
                                    + this->_client[client_index].get_nick_name()
                                    + "!"
                                    + this->_client[client_index].get_user_name()
                                    + "@localhost NICK :"
                                    + tokenized_message.params[0]
                                    + "\n";
        putstr_fd(server_answer, current_client_fd);
    }
    this->_client[client_index].set_nick_name(tokenized_message.params[0]);
}

void Server::commands_user(struct msg_tokens tokenized_message, int client_index)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
    //check for already taken username
    for (unsigned int i = 0; i < this->_client.size(); i++)
    {
        if (tokenized_message.params[0] == this->_client[i].get_user_name())
        {
            std::string already_in_use_message =    "466 <username> :"
                                                    + tokenized_message.params[0]
                                                    + " is already in use\n";

            putstr_fd(already_in_use_message, this->_client[i].get_sockfd());
            return ;
        }
    }

    this->_client[client_index].set_user_name(tokenized_message.params[0]);

    std::string welcome_message =   ":server 001 "
                                    + this->_client[client_index].get_nick_name()
                                    + " :Welcome to the IRC Network "
                                    + this->_client[client_index].get_nick_name()
                                    + "!"
                                    + this->_client[client_index].get_user_name()
                                    + "@localhost\n";

    putstr_fd(welcome_message, this->_client[client_index].get_sockfd());
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
        putstr_fd(":server 461 PRIVMSG :Not enough parameters\n", this->_client[client_index].get_sockfd());
        return;
    }
    std::string target = tokenized_message.params[0];
    if (target[0] == '#')
    {
        int channel_index = this->get_channel_index_through_name(target);
        if (!valid_channel_index(channel_index))
        {
            std::string server_message = ":server 403 " + target + " :No such channel\n";
            putstr_fd(server_message, this->_client[client_index].get_sockfd());
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
                putstr_fd(message, this->_client[recipient_index].get_sockfd());
            }
        }
    }
    else
    {
        int recipient_index = this->get_client_index_through_name(target);
        if (!valid_client_index(recipient_index))
        {
            std::string server_message = ":server 401 " + target + " :No such nick\n";
            putstr_fd(server_message, this->_client[client_index].get_sockfd());
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
        putstr_fd(message, this->_client[recipient_index].get_sockfd());
    }
}




void Server::commands_part(struct msg_tokens tokenized_message, int client_index)
{
	if (!valid_client_index(client_index) || _client[client_index].get_sockfd() == -1)
		return;
    //check if params vector is empty
    if (tokenized_message.params.empty() || tokenized_message.params[0].empty())
    {
        putstr_fd(":server 461 PART :Not enough parameters\n", this->_client[client_index].get_sockfd());
        return ;
    }

    int channel_index = this->get_channel_index_through_name(tokenized_message.params[0]);

    // channel doesnt exist
    if (!(valid_channel_index(channel_index)))
    {
        std::string nonexist_message = "403 " + tokenized_message.params[0] + " :No such channel\n";
        putstr_fd(nonexist_message, this->_client[client_index].get_sockfd());
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
                putstr_fd(message, this->_client[current_client_index].get_sockfd());
            }
            this->_channel[channel_index].remove_client_from_list(this->_client[client_index].get_nick_name());
            return ;
        }
    }
    // user not on channel
    std::string not_in_channel_message = "442 " + tokenized_message.params[0] + " :You're not on that channel\n";
    putstr_fd(not_in_channel_message, this->_client[client_index].get_sockfd());
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
                    putstr_fd(quit_msg, this->_client[target_index].get_sockfd());
            }
            channel.remove_client_from_list(nick);
        }
    }
    putstr_fd(quit_msg, this->_client[client_index].get_sockfd());
    disconnect_client(client_index);
}