
#include <iostream>
#include "Server.hpp"

void    check_input(int ac);

int main(int ac, char **av)
{
    Server server;

    check_input(ac);
    server.init(av);
    server.loop();
    server.end();
    return (0);
}

void     check_input(int ac)
{
    if (ac != 2)
        error("ERROR, no port provided", "human");
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

