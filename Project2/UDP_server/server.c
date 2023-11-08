// server code for UDP socket programming 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/select.h>

#define IP_PROTOCOL 0 
#define PORT_NO 15050 
#define NET_BUF_SIZE 32 
#define cipherKey 'S' 
#define sendrecvflag 0 
#define nofile "File Not Found!" 
#define WINDOW_SIZE 4
#define TIMEOUT 3 // in seconds

typedef struct {
    int seq_num;
    char data[NET_BUF_SIZE];
} Packet;

// function to clear buffer 
void clearBuf(char* b) { 
    int i; 
    for (i = 0; i < NET_BUF_SIZE; i++) 
        b[i] = '\0'; 
} 

// function to encrypt 
char Cipher(char ch) { 
    return ch ^ cipherKey; 
} 

// function sending file 
int sendFile(FILE* fp, char* buf, int s) { 
    int i, len; 
    if (fp == NULL) { 
        strcpy(buf, nofile); 
        len = strlen(nofile); 
        buf[len] = EOF; 
        for (i = 0; i <= len; i++) 
            buf[i] = Cipher(buf[i]); 
        return 1; 
    } 

    char ch, ch2; 
    for (i = 0; i < s; i++) { 
        ch = fgetc(fp); 
        ch2 = Cipher(ch); 
        buf[i] = ch2; 
        if (ch == EOF) 
            return 1; 
    } 
    return 0; 
} 

int main() { 
    int sockfd, nBytes; 
    struct sockaddr_in addr_con; 
    int addrlen = sizeof(addr_con); 
    addr_con.sin_family = AF_INET; 
    addr_con.sin_port = htons(PORT_NO); 
    addr_con.sin_addr.s_addr = INADDR_ANY; 
    char net_buf[NET_BUF_SIZE]; 
    FILE* fp; 

    // socket() 
    sockfd = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL); 

    if (sockfd < 0) 
        printf("\nfile descriptor not received!!\n"); 
    else
        printf("\nfile descriptor %d received\n", sockfd); 

    // bind() 
    if (bind(sockfd, (struct sockaddr*)&addr_con, sizeof(addr_con)) == 0) 
        printf("\nSuccessfully binded!\n"); 
    else
        printf("\nBinding Failed!\n"); 

    int base = 0;
    int next_seq_num = 0;
    Packet window[WINDOW_SIZE];
    int ack_received[WINDOW_SIZE];
    
    // Initialize variables
    for (int i = 0; i < WINDOW_SIZE; ++i) {
        ack_received[i] = 0;
    }

    while (1) { 
        printf("\nWaiting for file name...\n"); 

        // receive file name 
        clearBuf(net_buf); 

        nBytes = recvfrom(sockfd, net_buf, 
                        NET_BUF_SIZE, sendrecvflag, 
                        (struct sockaddr*)&addr_con, &addrlen); 

        fp = fopen(net_buf, "r"); 
        printf("\nFile Name Received: %s\n", net_buf); 
        if (fp == NULL) 
            printf("\nFile open failed!\n"); 
        else
            printf("\nFile Successfully opened!\n"); 

        while (1) { 
            // process 
            if (sendFile(fp, net_buf, NET_BUF_SIZE)) { 
                Packet ack_packet;
                ack_packet.seq_num = next_seq_num;
                strcpy(ack_packet.data, net_buf);
                sendto(sockfd, &ack_packet, sizeof(Packet), 
                    sendrecvflag, 
                    (struct sockaddr*)&addr_con, addrlen); 
                next_seq_num++;
                break; 
            } 

            // send 
            Packet packet;
            packet.seq_num = next_seq_num;
            strcpy(packet.data, net_buf);
            window[next_seq_num % WINDOW_SIZE] = packet;
            next_seq_num++;

            for (int i = base; i < next_seq_num; ++i) {
                sendto(sockfd, &(window[i % WINDOW_SIZE]), sizeof(Packet), 
                       sendrecvflag, (struct sockaddr*)&addr_con, addrlen);
            }

            // Set timeout for select() function
            fd_set fdset;
            FD_ZERO(&fdset);
            FD_SET(sockfd, &fdset);
            struct timeval timeout;
            timeout.tv_sec = TIMEOUT;
            timeout.tv_usec = 0;

            // Wait for ACK or timeout
            int activity = select(sockfd + 1, &fdset, NULL, NULL, &timeout);
            if (activity < 0) {
                perror("Select error");
                exit(EXIT_FAILURE);
            } else if (activity == 0) {
                // Timeout, resend packets
                continue;
            } else {
                // ACK received, update window and move base
                Packet ack_packet;
                recvfrom(sockfd, &ack_packet, sizeof(Packet), 
                         sendrecvflag, (struct sockaddr*)&addr_con, &addrlen);
                ack_received[ack_packet.seq_num % WINDOW_SIZE] = 1;

                while (ack_received[base % WINDOW_SIZE]) {
                    ack_received[base % WINDOW_SIZE] = 0;
                    base++;
                }
            }
        } 
        if (fp != NULL) 
            fclose(fp); 
    } 
    return 0; 
}
