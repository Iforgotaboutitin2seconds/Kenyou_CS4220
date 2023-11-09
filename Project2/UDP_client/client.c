#define _GNU_SOURCE
#define _XOPEN_SOURCE 600
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
#define TIMEOUT 2

typedef struct
{
	int seq_num;
	char data[BUFFER_SIZE];
} packet;

int check_timeout(struct timeval *start, int timeout)
{
	struct timeval now, diff;
	gettimeofday(&now, NULL);
	timersub(&now, start, &diff);
	if (diff.tv_sec > timeout || (diff.tv_sec == timeout && diff.tv_usec > 0))
	{
		return 1;
	}
	return 0;
}

int main()
{
	int sockfd;
	struct sockaddr_in servaddr;

	// Add a structure to keep track of the send times for each packet
	struct timeval send_times[WINDOW_SIZE];

	// Initialize all send times to 0
	for (int i = 0; i < WINDOW_SIZE; i++)
	{
		timerclear(&send_times[i]);
	}

	// Set the timeout duration (in seconds)
	int timeout_duration = TIMEOUT;

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
				int size = (i == num_packets - 1) ? n % BUFFER_SIZE : ((i + 1) * BUFFER_SIZE <= n) ? BUFFER_SIZE: n % BUFFER_SIZE;

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

				// Record the time the packet was sent
				gettimeofday(&send_times[next_seq_num % WINDOW_SIZE], NULL);
			}

			// Now, listen for ACKs with select() to avoid blocking
			fd_set readfds;
			struct timeval tv;

			FD_ZERO(&readfds);
			FD_SET(sockfd, &readfds);

			// Set timeout for select
			tv.tv_sec = 0;
			tv.tv_usec = 100000; // 100 ms for quick reaction time

			int rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);

			if (rv > 0)
			{ // There's something to read
				// Receive the ACK
				recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&servaddr, &len);

				// Check if the ack is for the base packet and update the base
				if (ack >= base)
				{
					base = ack + 1;
					// Update the send time for the new base (or clear if all caught up)
					if (base < next_seq_num)
					{
						gettimeofday(&send_times[base % WINDOW_SIZE], NULL);
					}
					else
					{
						timerclear(&send_times[base % WINDOW_SIZE]);
					}
				}
			}
			else if (rv == 0)
			{ // Timeout occurred
				// Check for each packet in the window if it has timed out
				for (int i = base; i < next_seq_num; i++)
				{
					if (check_timeout(&send_times[i % WINDOW_SIZE], timeout_duration))
					{
						// Timeout for packet i occurred, resend from base up to next_seq_num
						for (int j = base; j < next_seq_num; j++)
						{
							// Resend packet j
							// Make sure to reset the send time for packet j
							gettimeofday(&send_times[j % WINDOW_SIZE], NULL);
						}
						break; // Exit the for loop after resending
					}
				}
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
