
#include <iostream>
#include "Server/Server.hpp"

void    check_input(int ac, char **av);

int main(int ac, char **av)
{
    Server server;

    try
    {
        check_input(ac, av);
		if (!server.init(av))
		{
			std::cerr << "[ERROR] Server initialization failed. Exiting..." << std::endl;
			return EXIT_FAILURE;
		}
		server.loop();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    server.end();
    return (0);
}

void check_input(int ac, char **av)
{
	if (ac != 3)
		throw std::invalid_argument("[ERROR] provide a port and a password");

	std::string port_str = av[1];
	if (port_str.find_first_not_of("0123456789") != std::string::npos)
		throw std::invalid_argument("[ERROR] port must contain only numbers");
	int port = std::stoi(port_str);
	// Check the port number range < 1024 are used in the system
	if (port < 1024 || port > 65535)
		throw std::invalid_argument("[ERROR] port must be between 1024 and 65535");
}


//important sources: https://www.rfc-editor.org/rfc/rfc1459.txt

//  CLIENT:
//  info needed from client:all servers must have the
//  following information about all clients: the real name of the host
//  that the client is running on, the username of the client on that
//  host, and the server to which the client is connected.

//  CHANNEL OPERATORS:
//    A channel operator is identified by the '@' symbol next to their
//    nickname whenever it is associated with a channel (ie replies to the
//    NAMES, WHO and WHOIS commands).

//  CHANNELS:
//     Channels names are strings (beginning with a '&' or '#' character) of
//    length up to 200 characters.  Apart from the the requirement that the
//    first character being either '&' or '#'; the only restriction on a
//    channel name is that it may not contain any spaces (' '), a control G
//    (^G or ASCII 7), or a comma (',' which is used as a list item
//    separator by the protocol).

//dont forget fd leaks

// Using your reference client with your server must be similar to using it with any
// official IRC server. However, you only have to implement the following features:
// ◦ You must be able to authenticate, set a nickname, a username, join a channel,
// send and receive private messages using your reference client.
// ◦ All the messages sent from one client to a channel have to be forwarded to
// every other client that joined the channel.
// ◦ You must have operators and regular users.
// ◦ Then, you have to implement the commands that are specific to channel
// operators:
// ∗ KICK - Eject a client from the channel
// ∗ INVITE - Invite a client to a channel
// ∗ TOPIC - Change or view the channel topic
// ∗ MODE - Change the channel’s mode:
// · i: Set/remove Invite-only channel
// · t: Set/remove the restrictions of the TOPIC command to channel
// operators
// · k: Set/remove the channel key (password)
// · o: Give/take channel operator privilege
// 5
// ft_irc Internet Relay Chat
// · l: Set/remove the user limit to channel
// • Of course, you are expected to write a clean code.
