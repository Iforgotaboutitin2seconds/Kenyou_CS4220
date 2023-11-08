// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 8000
#define BUFFER_SIZE 1024
#define WINDOW_SIZE 4

typedef struct {
    int number;
    char data[BUFFER_SIZE];
} Packet;

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(sock, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    Packet window[WINDOW_SIZE];
    int window_start = 0;

    while (1) {
        // Receive a packet
        Packet packet;
        recvfrom(sock, &packet, sizeof(Packet), 0, NULL, NULL);

        // If the packet is the one we're expecting, slide the window and send an acknowledgment
        if (packet.number == window_start) {
            window_start++;
            sendto(sock, &packet, sizeof(Packet), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        }
    }

    close(sock);
    return 0;
}