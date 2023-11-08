// client code for UDP socket programming 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#define IP_PROTOCOL 0 
#define IP_ADDRESS "127.0.0.1" // localhost 
#define PORT_NO 15050 
#define NET_BUF_SIZE 32 
#define cipherKey 'S' 
#define sendrecvflag 0 
#define TIMEOUT 3 // in seconds

// function to clear buffer 
void clearBuf(char* b) 
{ 
	int i; 
	for (i = 0; i < NET_BUF_SIZE; i++) 
		b[i] = '\0'; 
} 

// function for decryption 
char Cipher(char ch) 
{ 
	return ch ^ cipherKey; 
} 

// function to receive file 
int recvFile(char* buf, int s) 
{ 
	int i; 
	char ch; 
	for (i = 0; i < s; i++) { 
		ch = buf[i]; 
		ch = Cipher(ch); 
		if (ch == EOF) 
			return 1; 
		else
			printf("%c", ch); 
	} 
	return 0; 
}

int sockfd;
struct sockaddr_in addr_con;
int addrlen = sizeof(addr_con);
char net_buf[NET_BUF_SIZE];
int base = 0;
int next_seq_num = 0;
int window_size = 4;
int timeout_flag = 0;

void timeout_handler(int signal) {
    timeout_flag = 1;
}

int main() 
{ 
	// socket() 
	sockfd = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL); 

	if (sockfd < 0) {
		perror("\nFile descriptor not received!!");
		exit(EXIT_FAILURE);
	}

	printf("\nFile descriptor %d received\n", sockfd);

	addr_con.sin_family = AF_INET; 
	addr_con.sin_port = htons(PORT_NO); 
	addr_con.sin_addr.s_addr = inet_addr(IP_ADDRESS); 

	FILE* fp; 
	struct sigaction sa;
    sa.sa_handler = timeout_handler;
    sigaction(SIGALRM, &sa, NULL);

	while (1) { 
		printf("\nPlease enter file name to receive:\n"); 
		scanf("%s", net_buf); 
		sendto(sockfd, net_buf, NET_BUF_SIZE, 
			sendrecvflag, (struct sockaddr*)&addr_con, 
			addrlen); 

		printf("\n---------Data Received---------\n"); 

		while (1) { 
			// receive 
			clearBuf(net_buf);
			alarm(TIMEOUT); // Set an alarm for TIMEOUT seconds
			timeout_flag = 0;

			while (!timeout_flag) {
				if (recvfrom(sockfd, net_buf, NET_BUF_SIZE, 
							 sendrecvflag, (struct sockaddr*)&addr_con, 
							 &addrlen) > 0) {
					alarm(0); // Reset the alarm upon successful receipt of data
					if (net_buf[0] == 'E' && net_buf[1] == 'O' && net_buf[2] == 'F') {
						goto end;
					}

					int seq_num = net_buf[0] - '0';
					if (seq_num >= base && seq_num < base + window_size) {
						printf("Received packet with sequence number: %d\n", seq_num);
						if (seq_num == base) {
							recvFile(net_buf + 1, NET_BUF_SIZE - 1);
							base++;
						}
					}

					int ack_packet = base;
					sendto(sockfd, &ack_packet, sizeof(int),
							sendrecvflag, (struct sockaddr*)&addr_con, addrlen);
				}
			}
		}
	}
	
	end:
	printf("\n-------------------------------\n"); 
	close(sockfd);
	return 0; 
}
