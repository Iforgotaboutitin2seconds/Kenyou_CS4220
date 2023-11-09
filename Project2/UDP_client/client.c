#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>

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

int main()
{
	int sockfd;
	struct sockaddr_in servaddr;
	struct timeval tv;
	fd_set readfds;
	int retval; // for select() result

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	servaddr.sin_port = htons(SERVER_PORT);

	char filename[BUFFER_SIZE];
	printf("Enter filename to send: ");
	scanf("%s", filename);

	FILE *file = fopen(filename, "r");
	if (!file)
	{
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}

	packet send_packet;
	int ack;
	int base = 0;
	int next_seq_num = 0;
	socklen_t len = sizeof(servaddr);

	// Read and send the file
	while (!feof(file))
	{
		if (next_seq_num < base + WINDOW_SIZE)
		{
			if (fgets(send_packet.data, BUFFER_SIZE, file) != NULL)
			{
				send_packet.seq_num = next_seq_num;

				printf("Sending packet with sequence number %d\n", send_packet.seq_num);
				sendto(sockfd, &send_packet, sizeof(send_packet), 0, (struct sockaddr *)&servaddr, len);
				next_seq_num++;
			}
		}

		// Set up timeout
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		tv.tv_sec = TIMEOUT;
		tv.tv_usec = 0;

		retval = select(sockfd + 1, &readfds, NULL, NULL, &tv);
		if (retval == -1)
		{
			perror("select()");
			exit(EXIT_FAILURE);
		}
		else if (retval)
		{
			// Data is available, read the ACK
			if (recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&servaddr, &len) > 0)
			{
				printf("Received ACK for packet: %d\n", ack);
				base = ack + 1;
			}
		}
		else
		{
			// Timeout occurred, resend the window of packets
			printf("Timeout occurred. Resending from packet %d\n", base);
			fseek(file, base * BUFFER_SIZE, SEEK_SET); // Rewind file to resend the window
			next_seq_num = base;
		}
	}

	fclose(file);
	close(sockfd);
	return 0;
}
