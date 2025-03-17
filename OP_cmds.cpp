#include "OP_cmds.hpp"
#include <algorithm>


void OP_commands::execute_operator_cmd(Server::MsgTokens tokenized_message, const std::string client_nickname, Channel &channel)
{
	(void)client_nickname;
	if (channel.get_name().empty()) // if channel name is empty
		return ;
	std::vector<std::string> opList = channel.get_operator_list();
	if (std::find(opList.begin(), opList.end(), client_nickname) == opList.end())
		return;
	
	std::string	command = tokenized_message.command;
	if (command == "KICK")
		return;//handle KICK
	else if (command == "INVITE")
		return;//handle Invite
	else if (command == "TOPIC")
		return;//handle topic
	else if (command == "MODE")
		return;//handle mode
	return;
}
