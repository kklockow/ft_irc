
#include <iostream>

int main(int ac, char **av)
{
    std::cout << "Hello World!" << std::endl;

    //check input

    //socket()  socket(AF_LOCAL, SOCK_STREAM, 0);
    // AF LOCAL is unix socker, sock stream because SOCK_DGRAM with 0 as third argument would use udp

    //bind()  with port is first arg (av[1])  -- gives name to socket (address)

    //listen()

    //accept() (blocks till connection with a client)


    //SEND AND RECEIVE DATA


    return (0);
}