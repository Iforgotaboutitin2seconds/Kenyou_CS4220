#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 12345
#define BUF_SIZE 4096

void fatal(char *string);

int main(int argc, char *argv[])
{
    int c, s, bytes;
    char buf[BUF_SIZE];
    struct hostent *h;
    struct sockaddr_in channel;

    if (argc != 4)
        fatal("Usage: client server-name file-to-send");

    h = gethostbyname(argv[1]);
    if (!h)
        fatal("gethostbyname");

    memset(&channel, 0, sizeof(channel));
    channel.sin_family = AF_INET;
    memcpy(&channel.sin_addr.s_addr, h->h_addr, h->h_length);
    channel.sin_port = htons(SERVER_PORT);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
        fatal("socket");

    c = connect(s, (struct sockaddr *)&channel, sizeof(channel));
    if (c < 0)
        fatal("connect failed");

    // Send the filename to the server
    write(s, argv[3], strlen(argv[3]) + 1);

    // Open and send the file to the server
    FILE *file = fopen(argv[3], "rb");
    if (!file)
        fatal("file not found");

    while ((bytes = fread(buf, 1, BUF_SIZE, file)) > 0)
    {
        write(s, buf, bytes);
    }

    fclose(file);
    close(s);
    return 0;
}

void fatal(char *string)
{
    printf("%s\n", string);
    exit(1);
}
