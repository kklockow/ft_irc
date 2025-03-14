
#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <iostream>
#include "../Utility/utils.hpp"
#include "../Client/Client.hpp"
#include "../Channel/Channel.hpp"
// #include "../OP_cmds.hpp"
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <sstream>

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
        void                            commands_message(struct msg_tokens tokenized_message, int client_index);
        void                            commands_part(struct msg_tokens tokenized_message, int client_index);
        int                             get_client_index_through_name(std::string client_name);
        int                             get_channel_index_through_name(std::string channel_name);
		msg_tokens                      error_message(std::string error_code, std::string message);
		void							send_error_message(int client_index, std::string error_code, std::string message);
		void							disconnect_client(int client_index);
		void                            commands_quit(struct msg_tokens tokenized_message, int client_index);

    public:
		Server();
		~Server();
        int                             get_sockfd();
        int                             get_port();
        struct sockaddr_in              get_server_address();
        void                            init(char **av);
        void                            loop();
        void                            end();
		typedef struct msg_tokens		MsgTokens;
};
