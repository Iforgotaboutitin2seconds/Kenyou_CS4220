#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 12345
#define BUF_SIZE 4096

int main(int argc, char *argv)
{
    int c, s, bytes;
    // buffer for incoming file
    char buf[BUF_SIZE];
    // info about server
    struct hostent *h;
    // holds IP address
    struct sockaddr_in channel;

    if (argc != 3)
        fatal("Usage: client server-name file-name");

    // look up host's IP address
    h = gethostbyname(argv[1]);
    if (!h)
        fatal("socket");
    memset(&channel, 0, sizeof(channel));
    channel.sin_family = AF_INET;
    memcpy(&channel.sin_addr.s_addr, h->h_addr, h->h_length);
    channel.sin_port = htons(SERVER_PORT);

    c = connect(s, (struct sockaddr *)&channel, sizeof(channel));
    if (c < 0)
        fatal("connect failed");

    // connection is now established. Send file name inlcuding 0 byte at end.
    write(s, argv[2], strlen(argv[2]) + 1);

    // go get the file and write it o standard output
    while (1)
    {
        // read from socket
        bytes = read(s, buf, BUF_SIZE);

        // check for end of file
        if (bytes <= 0)
            exit(0);

        // write to standard output
        write(1, buf, bytes);
    }
}

fatal(char *string)
{
    printf("%s\n", string);
    exit(1);
}