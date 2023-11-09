#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8000
#define BUFFER_SIZE 1024
#define WINDOW_SIZE 4
#define TIMEOUT_SEC 2 // Timer value in seconds

typedef struct
{
    int seq_num;
    char data[BUFFER_SIZE];
} packet;

// Function to set the socket timeout
void set_socket_timeout(int sockfd, int sec) {
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    packet send_packet;
    int ack;
    int base = 0;
    int next_seq_num = 0;
    struct timeval start_time, end_time, time_diff;
    socklen_t len = sizeof(servaddr);

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // Initialize server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(SERVER_PORT);

    // Set socket timeout for receiving ACKs
    set_socket_timeout(sockfd, TIMEOUT_SEC);

    // Prepare data to be sent (for testing, we'll just use a simple string)
    char data_to_send[] = "Hello from client!";
    int data_len = strlen(data_to_send) + 1; // +1 for the null-terminator

    while (base < data_len) {
        // Send window of packets
        while (next_seq_num < base + WINDOW_SIZE && next_seq_num < data_len) {
            memset(&send_packet, 0, sizeof(send_packet)); // Clear out the packet structure
            send_packet.seq_num = next_seq_num;
            // Copy a portion of data_to_send into send_packet.data, ensuring we don't exceed BUFFER_SIZE
            strncpy(send_packet.data, data_to_send + next_seq_num, BUFFER_SIZE);

            // Send the packet
            sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr *)&servaddr, len);
            if (base == next_seq_num) {
                gettimeofday(&start_time, NULL); // Start timer for the base packet
            }
            next_seq_num++;
        }

        // Wait for ACK or timeout
        int recv_status = recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&servaddr, &len);

        if (recv_status > 0 && ack >= base) {
            // Move the window
            base = ack + 1;

            // Restart timer if there are still more packets to send
            if (base != next_seq_num) {
                gettimeofday(&start_time, NULL);
            }
        } else if (recv_status < 0) {
            // Timeout or error
            printf("Timeout or error in receiving ACK. Retransmitting from seq_num: %d\n", base);
            // Retransmit all packets in window
            next_seq_num = base;
        }

        // Check if timeout has occurred for base packet
        gettimeofday(&end_time, NULL);
        timersub(&end_time, &start_time, &time_diff);
        if (time_diff.tv_sec >= TIMEOUT_SEC) {
            // Timeout occurred, retransmit all packets in the window
            printf("Timeout for packet with seq_num: %d\n", base);
            next_seq_num = base;
        }
    }

    // Close the socket
    close(sockfd);

    return 0;
}
