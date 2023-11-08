#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8000
#define BUFFER_SIZE 1024
#define WINDOW_SIZE 4
#define TIMEOUT 5

int sock;
struct sockaddr_in server_addr;
char buffer[BUFFER_SIZE];
int bytes_read;
int seq_num = 0;
int window_base = 0;
int window_end = WINDOW_SIZE - 1;
bool is_timer_running = false;

void start_timer() {
	is_timer_running = true;
	alarm(TIMEOUT);
}

void stop_timer() {
	is_timer_running = false;
	alarm(0);
}

void handle_timeout(int sig) {
	printf("Timeout occurred. Resending packets.\n");
	for (int i = window_base; i <= window_end; i++) {
		sendto(sock, buffer + i * BUFFER_SIZE, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
	}
	start_timer();
}

void send_packet(int seq_num) {
	sendto(sock, buffer + seq_num * BUFFER_SIZE, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if (seq_num == window_base && !is_timer_running) {
		start_timer();
	}
}

int main() {
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

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

	while ((bytes_read = fread(buffer, 1, BUFFER_SIZE * WINDOW_SIZE, fp)) > 0) {
		window_base = 0;
		window_end = WINDOW_SIZE - 1;
		seq_num = 0;
		while (window_base <= bytes_read / BUFFER_SIZE) {
			for (int i = window_base; i <= window_end && i <= bytes_read / BUFFER_SIZE; i++) {
				send_packet(i);
			}
			int expected_ack = window_base;
			while (expected_ack <= window_end && expected_ack <= bytes_read / BUFFER_SIZE) {
				struct sockaddr_in client_addr;
				socklen_t client_addr_len = sizeof(client_addr);
				int ack_num;
				recvfrom(sock, &ack_num, sizeof(ack_num), MSG_DONTWAIT, (struct sockaddr*)&client_addr, &client_addr_len);
				if (ack_num == expected_ack) {
					expected_ack++;
					window_base++;
					window_end++;
					stop_timer();
				}
			}
		}
	}

	fclose(fp);
	close(sock);
	break;
	return 0;
}
