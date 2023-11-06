#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024
#define WINDOW_SIZE 4
#define TIMEOUT 5

struct packet {
    int seq_num;
    char data[BUFFER_SIZE];
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];

    // Open the file for reading
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("fopen");
        return 1;
    }

    // Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Set up the server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        return 1;
    }

    // Initialize variables for Go Back-N protocol
    int base = 0;
    int next_seq_num = 0;
    int expected_ack = 0;
    struct packet window[WINDOW_SIZE];
    bool sent[WINDOW_SIZE] = {false};
    bool received_ack[WINDOW_SIZE] = {false};
    struct timeval timer;
    timer.tv_sec = TIMEOUT;
    timer.tv_usec = 0;

    // Send the file to the server
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        // Wait for ACKs
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        int select_result = select(sockfd + 1, &readfds, NULL, NULL, &timer);
        if (select_result == -1) {
            perror("select");
            return 1;
        } else if (select_result == 0) {
            // Timeout occurred, resend packets
            for (int i = base; i < next_seq_num; i++) {
                if (!received_ack[i % WINDOW_SIZE]) {
                    if (sendto(sockfd, &window[i % WINDOW_SIZE], sizeof(struct packet), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                        perror("sendto");
                        return 1;
                    }
                }
            }
            timer.tv_sec = TIMEOUT;
            timer.tv_usec = 0;
            continue;
        }

        // Receive ACKs
        struct packet ack_packet;
        ssize_t recv_result = recv(sockfd, &ack_packet, sizeof(struct packet), 0);
        if (recv_result < 0) {
            perror("recv");
            return 1;
        } else if (recv_result == 0) {
            printf("Server closed connection\n");
            return 1;
        }

        // Update variables based on received ACK
        int ack_num = ack_packet.seq_num;
        if (ack_num >= base && ack_num < next_seq_num) {
            received_ack[ack_num % WINDOW_SIZE] = true;
            while (received_ack[base % WINDOW_SIZE]) {
                received_ack[base % WINDOW_SIZE] = false;
                sent[base % WINDOW_SIZE] = false;
                base++;
            }
            timer.tv_sec = TIMEOUT;
            timer.tv_usec = 0;
        }

        // Send packets
        if (next_seq_num < base + WINDOW_SIZE) {
            struct packet data_packet;
            data_packet.seq_num = next_seq_num;
            memcpy(data_packet.data, buffer, bytes_read);
            memcpy(window[next_seq_num % WINDOW_SIZE].data, buffer, bytes_read);
            window[next_seq_num % WINDOW_SIZE].seq_num = next_seq_num;
            if (sendto(sockfd, &data_packet, sizeof(struct packet), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("sendto");
                return 1;
            }
            sent[next_seq_num % WINDOW_SIZE] = true;
            next_seq_num++;
        }
    }

    // Wait for final ACKs
    while (base < next_seq_num) {
        // Wait for ACKs
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        int select_result = select(sockfd + 1, &readfds, NULL, NULL, &timer);
        if (select_result == -1) {
            perror("select");
            return 1;
        } else if (select_result == 0) {
            // Timeout occurred, resend packets
            for (int i = base; i < next_seq_num; i++) {
                if (!received_ack[i % WINDOW_SIZE]) {
                    if (sendto(sockfd, &window[i % WINDOW_SIZE], sizeof(struct packet), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                        perror("sendto");
                        return 1;
                    }
                }
            }
            timer.tv_sec = TIMEOUT;
            timer.tv_usec = 0;
            continue;
        }

        // Receive ACKs
        struct packet ack_packet;
        ssize_t recv_result = recv(sockfd, &ack_packet, sizeof(struct packet), 0);
        if (recv_result < 0) {
            perror("recv");
            return 1;
        } else if (recv_result == 0) {
            printf("Server closed connection\n");
            return 1;
        }

        // Update variables based on received ACK
        int ack_num = ack_packet.seq_num;
        if (ack_num >= base && ack_num < next_seq_num) {
            received_ack[ack_num % WINDOW_SIZE] = true;
            while (received_ack[base % WINDOW_SIZE]) {
                received_ack[base % WINDOW_SIZE] = false;
                sent[base % WINDOW_SIZE] = false;
                base++;
            }
            timer.tv_sec = TIMEOUT;
            timer.tv_usec = 0;
        }
    }

    // Close the file and socket
    fclose(file);
    close(sockfd);

    return 0;
}
