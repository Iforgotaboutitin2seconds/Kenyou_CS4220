#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8000
#define BUFFER_SIZE 1024

typedef struct
{
    int seq_num;
    char data[BUFFER_SIZE];
} packet; 

int main()
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // Initialize server address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind socket to server address
    bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    packet recv_packet;
    int len = sizeof(cliaddr);
    int expected_seq_num = 0;

    while (1)
    {
        // Receive packet from client
        recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0, (struct sockaddr *)&cliaddr, &len);

        // Check sequence number
        if (recv_packet.seq_num == expected_seq_num)
        {
            printf("Received packet with sequence number %d: %s\n", recv_packet.seq_num, recv_packet.data);

            // Send ACK
            sendto(sockfd, &expected_seq_num, sizeof(expected_seq_num), 0, (struct sockaddr *)&cliaddr, len);
            expected_seq_num++;
        }
        else
        {
            printf("Received out of order packet. Expected: %d, Received: %d\n", expected_seq_num, recv_packet.seq_num);

            // Send duplicate ACK for the last in-order packet
            int dup_ack = expected_seq_num - 1;
            sendto(sockfd, &dup_ack, sizeof(dup_ack), 0, (struct sockaddr *)&cliaddr, len);
        }
    }

    return 0;
}
