#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(0); // Define your port number
    server_address.sin_addr.s_addr = INADDR_ANY;  // Bind to any available interface

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 1) == -1)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    int client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1)
    {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0)
    {
        perror("Receive failed");
    }
    else
    {
        buffer[bytes_received] = '\0'; // Null-terminate the received data
        printf("Received: %s\n", buffer);

        // You can also send a response back to the client using send()
    }

    close(client_socket);
    close(server_socket);

    return 0;
}
