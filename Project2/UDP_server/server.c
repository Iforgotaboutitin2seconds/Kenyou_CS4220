#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>

#define BUF_SIZE 1024
#define PORT 8000
#define WINDOW_SIZE 4
#define TIMEOUT 5

struct sigaction sa;

typedef struct
{
    int seq_num;
    char data[BUF_SIZE];
} packet;

void timeout_handler(int signum)
{
    printf("Timeout occurred.\n");
}

int main()
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUF_SIZE];
    int len, n;

    // Set up the signal handler for timeouts using sigaction
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = timeout_handler;
    sigaction(SIGALRM, &sa, NULL);

    // create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // bind socket to address
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // receive message from client
    len = sizeof(client_addr);
    n = recvfrom(sockfd, (char *)buffer, BUF_SIZE, MSG_WAITALL, (struct sockaddr *)&client_addr, &len);
    buffer[n] = '\0';
    printf("Received message from client: %s\n", buffer);

    // send message back to client using Go-Back-N protocol
    int base = 0;
    int next_seq_num = 0;
    packet window[WINDOW_SIZE];
    bool ack_received[WINDOW_SIZE] = {false};
    while (base < strlen(buffer))
    {
        // send packets in window
        for (int i = base; i < base + WINDOW_SIZE && i < strlen(buffer); i++)
        {
            packet p;
            p.seq_num = i;
            strncpy(p.data, buffer + i, BUF_SIZE);
            sendto(sockfd, (const char *)&p, sizeof(p), MSG_CONFIRM, (const struct sockaddr *)&client_addr, len);
            printf("Sent packet with sequence number %d.\n", p.seq_num);
            if (i == next_seq_num)
            {
                alarm(TIMEOUT);
            }
            next_seq_num++;
        }

        // Setup the alarm before starting to receive ACKs
        alarm(TIMEOUT);

        // receive acknowledgments
        while (base < next_seq_num) {
            packet ack;
            int ack_len = recvfrom(sockfd, (char *)&ack, sizeof(ack), 0, (struct sockaddr *)&client_addr, &len);
            if (ack_len > 0) {
                printf("Received acknowledgment for sequence number %d.\n", ack.seq_num);
                if (ack.seq_num >= base && ack.seq_num < base + WINDOW_SIZE) {
                    ack_received[ack.seq_num - base] = true;
                }
                if (ack.seq_num == base) {
                    // slide window
                    int i;
                    for (i = 0; i < WINDOW_SIZE && ack_received[i]; i++) {
                        ack_received[i] = false;
                    }
                    base += i;
                    alarm(0); // Cancel the timer if ACK is received
                }
            } else if (errno != EWOULDBLOCK) {
                perror("recvfrom");
                exit(EXIT_FAILURE);
            }
        }

        // handle timeout
        if (base < next_seq_num)
        {
            signal(SIGALRM, timeout_handler);
            alarm(TIMEOUT);
            pause();
        }
    }

    close(sockfd);
    return 0;
}