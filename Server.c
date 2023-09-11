#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVER_PORT 12345
#define BUF_SIZE 4096
#define QUEUE_SIZE 10

void fatal(char *string);

int main(int argc, chat *argv[])
{
    int s, b, l, fd, sa, bytes, on = 1;
    char buf[BUF_SIZE];
    struct sockaddr_in channel;

    // build address structure to bind to socket
    memset(&channel, 0, sizeof(channel));
    channel.sin_family = AF_INET;
    channel.sin_addr.saddr = htonl(NADDR_ANY);
    channel.sin_port = htons(SERVER_PORT);

    // passive open, wait for connection
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0)
        fatal("socket failed");
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

    b = bind(s, (struct sockaddr *)&channel, sizeof(channel));
    if (b < 0)
        fatal("bind failed");

    l = listen(s, QUEUE_SIZE);
    if (l < 0)
        fatal("listen failed");

    // socket is now set up and bound. Wait for connection and process it.
    while (1)
    {
        // block for connection request
        sa = accpet(s, 0, 0);
        if (sa < 0)
            fatal("accept failed");

        // read file name from socket
        read(sa, buf, BUF_SIZE);

        // get and return file
        fd = open(buf, O_RDNOLY);
        if (fd < 0)
            fatal("open failed");

        while (1)
        {
            bytes = read(fd, buf, BUF_SIZE);
            if (bytes <= 0)
                break;
            write(sa, buf, bytes);
        }
        // close file
        close(fd);
        // close connection
        close(sa);
    }
}

fatal(char *string)
{
    printf("%s\n", string);
    exit(1);
}