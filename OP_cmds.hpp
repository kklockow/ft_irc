#pragma once

#include "Channel/Channel.hpp"
#include "Server/Server.hpp"

class OP_commands
{
	public:
		static void	execute_operator_cmd(Server::MsgTokens tokenized_message, const std::string client_nickname, Channel &channel);

};