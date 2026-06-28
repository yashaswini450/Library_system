#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <fcntl.h>

#include "../auth.h"
#include "ser.h"

#define PORT 8080
#define MAX_CONNECTIONS 5

// Creates all required .bin files if they don't already exist.
// Needed on fresh clones where db_files/ is empty and fopen("rb+") would fail.
static void init_db_files(void) {
    const char *files[] = { admAccs, memAccs, booksCol, allocList };
    int n = sizeof(files) / sizeof(files[0]);
    for (int i = 0; i < n; i++) {
        FILE *f = fopen(files[i], "ab"); // "ab" creates if missing, never truncates
        if (f) fclose(f);
        else { perror(files[i]); exit(EXIT_FAILURE); }
    }
    printf("Database files initialised.\n");
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }
#ifdef SO_REUSEPORT
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt SO_REUSEPORT");
        exit(EXIT_FAILURE);
    }
#endif

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CONNECTIONS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    init_db_files();
    printf("Server running at 127.0.0.1:%d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Connection accepted from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        int pid = fork();
        if (pid == 0) {
            close(server_fd);
            handle_connection(new_socket, &address);
            exit(0);
        } else if (pid > 0) {
            close(new_socket);
        } else {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
