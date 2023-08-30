#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    // Open the file to send
    FILE *file_to_send = fopen("file_to_send.txt", "rb");
    if (!file_to_send) {
        perror("File open failed");
        exit(EXIT_FAILURE);
    }

    // Send file
    while (1) {
        int bytes_read = fread(buffer, 1, BUFFER_SIZE, file_to_send);
        if (bytes_read <= 0) {
            break;
        }
        send(client_socket, buffer, bytes_read, 0);
    }
    fclose(file_to_send);

    printf("File sent\n");

    close(client_socket);

    return 0;
}
