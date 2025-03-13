
#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <iostream>
#include "utils.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <sstream>
#include <regex>

class Server
{
    private:
		struct msg_tokens
		{
			std::string                 prefix;
			std::string                 command;
			std::vector<std::string>    params;
			std::string                 trailing;
		};

        bool                            running;
        int                             _sockfd;
        int                             _port;
        struct sockaddr_in              _server_address;
        std::vector<Client>             _client;
        std::vector<struct pollfd>      _poll_fd;
        std::vector<Channel>            _channel;
		std::string						_password;
        void                            create_socket();
        void                            fill_socket_struct();
        void                            bind_server_address();
        void                            init_poll_struct(int fd);
        void                            accept_client();
        void                            receive_data(int client_index);
        void                            handle_data(int client_index);
        msg_tokens                      parse_message_line(std::string line);
        void                            execute_command(struct msg_tokens tokenized_message, int client_index);
		bool							authenticateClient(const msg_tokens &tokenized_message, int client_index);
        void                            commands_join(struct msg_tokens tokenized_message, int client_index);
        void                            commands_join_message_clients(std::string channel_name);
        void                            commands_user(struct msg_tokens tokenized_message, int client_index);
        void                            commands_nick(struct msg_tokens tokenized_message, int client_index);
        int                             get_client_index_through_name(std::string client_name);
        int                             get_channel_index_through_name(std::string channel_name);
		msg_tokens                      error_message(std::string error_code, std::string message);

    public:
        int                             get_sockfd();
        int                             get_port();
        struct sockaddr_in              get_server_address();
        void                            init(char **av);
        void                            loop();
        void                            end();
};