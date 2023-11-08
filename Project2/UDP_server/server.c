#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define PORT 8000

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUF_SIZE];
    int len, n;

    // create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // bind socket to address
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // receive message from client
    len = sizeof(client_addr);
    n = recvfrom(sockfd, (char *)buffer, BUF_SIZE, MSG_WAITALL, (struct sockaddr *)&client_addr, &len);
    buffer[n] = '\0';
    printf("Received message from client: %s\n", buffer);

    // send message back to client
    sendto(sockfd, (const char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&client_addr, len);
    printf("Message sent back to client.\n");

    close(sockfd);
    return 0;
}
