#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888
#define BUFFER_SIZE 1024

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

	char buffer[BUFFER_SIZE];
	int bytes_read;
	while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
		sendto(sock, buffer, bytes_read, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
	}

	fclose(fp);
	close(sock);
	return 0;
}
