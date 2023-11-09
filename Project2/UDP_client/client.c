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

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	servaddr.sin_port = htons(SERVER_PORT);

	packet send_packet;
	int ack;
	int base = 0;
	int next_seq_num = 0;
	socklen_t len = sizeof(servaddr);

	char filename[100];
	printf("Enter the filename to send: ");
	scanf("%s", filename);

	int fd = open(filename, O_RDONLY);
	if (fd == -1)
	{
		printf("Error opening file\n");
		return 1;
	}

	int n;
	off_t offset = 0;
	while ((n = pread(fd, send_packet.data, BUFFER_SIZE, offset)) > 0)
	{
		int num_packets = (n + BUFFER_SIZE - 1) / BUFFER_SIZE;
		for (int i = 0; i < num_packets; i++)
		{
			if (next_seq_num < base + WINDOW_SIZE)
			{
				send_packet.seq_num = next_seq_num;
				int size = (i == num_packets - 1) ? n % BUFFER_SIZE : ((i + 1) * BUFFER_SIZE <= n) ? BUFFER_SIZE
																								   : n % BUFFER_SIZE;
				printf("Sending packet with sequence number %d and size %d\n", send_packet.seq_num, size);
				memcpy(send_packet.data, &send_packet.data[i * BUFFER_SIZE], size);
				sendto(sockfd,
					   &send_packet,
					   sizeof(send_packet),
					   0,
					   (struct sockaddr *)&servaddr,
					   sizeof(servaddr));
				next_seq_num++;
			}
		}
		offset += n;
	}

	close(fd);
	close(sockfd);

	return 0;
}
