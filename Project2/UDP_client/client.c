#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8000
#define BUFFER_SIZE 1024
#define WINDOW_SIZE 4
#define TIMEOUT 2 // seconds

typedef struct
{
	int seq_num;
	char data[BUFFER_SIZE];
} packet;

int main()
{
	int sockfd;
	struct sockaddr_in servaddr;

	// Create a UDP socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	// Set server address to 0
	memset(&servaddr, 0, sizeof(servaddr));

	// Set server address family to IPv4
	servaddr.sin_family = AF_INET;

	// Set server IP address
	servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	// Set server port number
	servaddr.sin_port = htons(SERVER_PORT);

	// Create a packet to send
	packet send_packet;

	// Initialize variables
	int ack;
	int base = 0;
	int next_seq_num = 0;
	socklen_t len = sizeof(servaddr);

	// Get filename from user
	char filename[100];
	printf("Enter the filename to send: ");
	scanf("%s", filename);

	// Open file for reading
	int fd = open(filename, O_RDONLY);
	if (fd == -1)
	{
		printf("Error opening file\n");
		return 1;
	}

	// Read file and send packets
	int n;
	off_t offset = 0;
	struct timeval tv;
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	while ((n = pread(fd, send_packet.data, BUFFER_SIZE, offset)) > 0)
	{
		// Calculate number of packets needed to send the data
		int num_packets = (n + BUFFER_SIZE - 1) / BUFFER_SIZE;

		// Send packets
		for (int i = 0; i < num_packets; i++)
		{
			// Send only if the window is not full
			if (next_seq_num < base + WINDOW_SIZE)
			{
				// Set sequence number
				send_packet.seq_num = next_seq_num;

				// Calculate packet size
				int size = (i == num_packets - 1) ? n % BUFFER_SIZE : ((i + 1) * BUFFER_SIZE <= n) ? BUFFER_SIZE
																								   : n % BUFFER_SIZE;

				// Copy data to packet
				memcpy(send_packet.data, &send_packet.data[i * BUFFER_SIZE], size);

				// Send packet
				sendto(sockfd,
					   &send_packet,
					   sizeof(send_packet),
					   0,
					   (struct sockaddr *)&servaddr,
					   sizeof(servaddr));

				// Increment sequence number
				next_seq_num++;

				// Wait for acknowledgement
				while (recvfrom(sockfd,
								&ack,
								sizeof(ack),
								MSG_WAITALL,
								(struct sockaddr *)&servaddr,
								&len) <= 0)
				{
					// Resend packets if acknowledgement not received within TIMEOUT seconds
					sendto(sockfd,
						   &send_packet,
						   sizeof(send_packet),
						   0,
						   (struct sockaddr *)&servaddr,
						   sizeof(servaddr));
				}

				// Update base
				base = ack + 1;
			}
		}

		// Update offset
		offset += n;
	}

	// Close file and socket
	close(fd);
	close(sockfd);

	return 0;
}
