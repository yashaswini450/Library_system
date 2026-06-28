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

#define MAX_BUFFSIZE 1024

#define TESTING_MODE 0

#if TESTING_MODE
    #define send my_send
    #define recv my_recv
#endif // BASICALLY A MOCKING OF THE ACTUAL send()/recv() OF SOCKET PROGRAMMING

void viewBooksInLibrary(int client_socket, int testing_mode) {
    char buffer[MAX_BUFFSIZE] = {0};
    FILE *bookFile;
    int fd;
    struct Book book;

    // Open the correct file based on testing mode
    if (testing_mode) {
        // Open a temporary or mock test file
        bookFile = fopen(test_booksCol, "wb+"); // Temporary test file
    } else {
        bookFile = fopen(booksCol, "rb+");  // Regular file
    }

    fd = fileno(bookFile);
    lock_file(fd, F_RDLCK); // Acquire read lock
    fseek(bookFile, 0, SEEK_SET);

    strcpy(buffer, "Books in library:\n");
    while (fread(&book, sizeof(struct Book), 1, bookFile)) {
        if (book.delete == 0) {
            char temp[100];
            sprintf(temp, "Class ID: %s, Name: %s, Copies: %d\n", book.class_id, book.name, book.copies);
            strcat(buffer, temp);
        }
    }
    lock_file(fd, F_UNLCK); // Release lock
    fclose(bookFile);

    send(client_socket, buffer, MAX_BUFFSIZE, 0);
}

void viewCurrentIssues(int client_socket, struct Account *acc, int testing_mode) {
    char buffer[MAX_BUFFSIZE] = {0};
    FILE *allocFile;
    int fd;
    struct Allocation allocation;

    // Open the correct file based on testing mode
    if (testing_mode) {
        // Open a temporary or mock test allocation file
        allocFile = fopen(test_allocList, "wb+");  // Temporary test file
    } else {
        allocFile = fopen(allocList, "rb+");  // Regular file
    }

    if (allocFile == NULL) {
        send(client_socket, "Error opening allocation file.\n", strlen("Error opening allocation file.\n"), 0);
        return;
    }

    fd = fileno(allocFile);
    lock_file(fd, F_RDLCK); // Acquire read lock
    fseek(allocFile, 0, SEEK_SET);

    strcpy(buffer, "Your current allocations:\n");
    while (fread(&allocation, sizeof(struct Allocation), 1, allocFile)) {
        if (allocation.delete == 0 && strcmp(allocation.name, acc->username) == 0) {
            char temp[200];
            sprintf(temp, "Class ID: %s, Date of Issue: %s, Date of Return: %s\n",
                    allocation.class_id, allocation.dateOfIssue, allocation.dateOfReturn);
            strcat(buffer, temp);
        }
    }
    lock_file(fd, F_UNLCK); // Release lock
    fclose(allocFile);

    send(client_socket, buffer, MAX_BUFFSIZE, 0);
}

void exitMemberPanel(int client_socket) {
    send(client_socket, "Exiting member panel.\n", strlen("Exiting member panel.\n"), 0);
}
