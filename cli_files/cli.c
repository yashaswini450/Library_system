#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../auth.h"
#include "cli.h"

#define PORT 8080

// Main function to create a client socket, connect to server, and handle user choices
int main() {
    int client_socket;
    struct sockaddr_in server_address;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation error");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    const char *host = getenv("SERVER_HOST");
    if (!host) host = "127.0.0.1";
    server_address.sin_addr.s_addr = inet_addr(host);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection error");
        return -1;
    }

    char choice[2];
    char buffer[MAX_BUFFSIZE];

    recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive initial server message
    printf("%s", buffer);
    scanf("%s", choice);
    send(client_socket, choice, strlen(choice), 0);

    if (strcmp(choice, "1") == 0) {
        registerAccount_client(client_socket); // Handle account registration
    } else if (strcmp(choice, "2") == 0) {
        login_client(client_socket); // Handle login
    } else {
        printf("Invalid choice.\n");
    }

    close(client_socket); // Close the socket
    return 0;
}
