// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
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
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    char filename[256];
    printf("Enter filename: ");
    fgets(filename, 256, stdin);
    filename[strcspn(filename, "\n")] = 0; // remove newline character

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("file open failed");
        exit(EXIT_FAILURE);
    }

    Packet window[WINDOW_SIZE];
    int window_start = 0;
    int packet_number = 0;

    while (!feof(fp)) {
        // Fill the window with packets
        for (int i = 0; i < WINDOW_SIZE && !feof(fp); i++) {
            window[i].number = packet_number++;
            fread(window[i].data, 1, BUFFER_SIZE, fp);
        }

        // Send all packets in the window
        for (int i = 0; i < WINDOW_SIZE; i++) {
            sendto(sock, &window[i], sizeof(Packet), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        }

        // Wait for acknowledgment
        Packet ack;
        recvfrom(sock, &ack, sizeof(Packet), 0, NULL, NULL);

        // If acknowledgment for the first packet in the window is received, slide the window
        if (ack.number == window_start) {
            window_start++;
        }
    }

    fclose(fp);
    close(sock);
    return 0;
}