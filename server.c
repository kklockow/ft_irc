/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;


     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));


     portno = atoi(argv[1]);


     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);


     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");

     listen(sockfd,5);
     clilen = sizeof(cli_addr);

    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        int count = 0;
        struct pollfd s_poll;

        memset(&s_poll, 0, sizeof(s_poll));
        s_poll.fd = newsockfd;
        s_poll.events = POLLIN;


        if (newsockfd < 0)
            error("ERROR on accept");
        while (1)
        {
            if (poll(&s_poll, 1, 100) == 1)
            {
                bzero(buffer,256);
                n = read(newsockfd,buffer,255);
                if (n < 0) error("ERROR reading from socket");
                printf("Here is the message: %s\n",buffer);
                n = write(newsockfd,"I got your message",18);
                if (n < 0) error("ERROR writing to socket");
                break ;
            }
            else
                count++;
        }
        //buffer has newline char
        printf("Took %d   buffer is %s\n", count * 100, buffer);
        close(newsockfd);
        if (strncmp(buffer, "stop", 4) == 0)
            break ;
    }
    close(sockfd);
    return 0;
}