#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buf[BUF_SIZE];
    FILE *fp;
    int seq_num = 0; // variable to track sequence numbers

    // Create UDP socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Bind socket to port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Receive file from client
    if ((fp = fopen(argv[2], "wb")) == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    while (1) {
        memset(buf, 0, BUF_SIZE);
        if (recvfrom(sock, buf, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len) < 0) {
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }
        if (strcmp(buf, "END") == 0) {
            break;
        }
        fwrite(buf, sizeof(char), strlen(buf), fp);
        seq_num++;
    }
    fclose(fp);

    close(sock);
    return 0;
}
