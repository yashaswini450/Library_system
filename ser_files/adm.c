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
#endif // BASICALLY A MOCKING OF THE ACTUAL send()/recv() OF SOCKET PROGRAMMING. THIS OVERRIDES send() AND recv() DEFINED BY STD LIBRARY.

void seeAllBooks(int client_socket, int testing_mode) {
    char buffer[MAX_BUFFSIZE] = {0};
    FILE *bookFile;
    int fd;
    struct Book book;

    // Open the correct file based on testing mode
    if (testing_mode) {
        // Open a temporary or mock test book file for testing
        bookFile = fopen(test_booksCol, "rb+"); // Temporary test file
    } else {
        bookFile = fopen(booksCol, "rb+");  // Regular file
    }
    if (bookFile == NULL) {
        send(client_socket, "Error opening book file.\n", MAX_BUFFSIZE, 0);
        return;
    }

    fd = fileno(bookFile);
    lock_file(fd, F_RDLCK); // Acquire read lock
    fseek(bookFile, 0, SEEK_SET);

    strcpy(buffer, "Books and available copies:\n");
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

void seeAllocations(int client_socket, int testing_mode) {
    char buffer[MAX_BUFFSIZE] = {0};
    FILE *allocFile;
    int fd;
    struct Allocation allocation;

    // Open the correct file based on testing mode
    if (testing_mode) {
        // Open a temporary or mock test allocation file for testing
        allocFile = fopen(test_allocList, "rb+");  // Temporary test file
    } else {
        allocFile = fopen(allocList, "rb+");  // Regular file
    }

    if (allocFile == NULL) {
        send(client_socket, "Error opening allocation file.\n", MAX_BUFFSIZE, 0);
        return;
    }

    fd = fileno(allocFile);
    lock_file(fd, F_RDLCK); // Acquire read lock
    fseek(allocFile, 0, SEEK_SET);

    memset(buffer, '\0', sizeof(buffer));
    strcpy(buffer, "Current allocations:\n");
    while (fread(&allocation, sizeof(struct Allocation), 1, allocFile)) {
        if (allocation.delete == 0) {
            char temp[200];
            sprintf(temp, "Class ID: %s, Member Name: %s, Date of Issue: %s, Date of Return: %s\n",
                    allocation.class_id, allocation.name, allocation.dateOfIssue, allocation.dateOfReturn);
            strcat(buffer, temp);
        }
    }
    lock_file(fd, F_UNLCK); // Release lock
    fclose(allocFile);

    send(client_socket, buffer, MAX_BUFFSIZE, 0);
}

void addBook(int client_socket, int testing_mode) {
    struct Book book;
    char buffer[MAX_BUFFSIZE] = {0};
    FILE *bookFile;
    int fd;

    send(client_socket, "Enter class ID: ", MAX_BUFFSIZE, 0);
    recv(client_socket, book.class_id, sizeof(book.class_id), 0);

    send(client_socket, "Enter book name: ", MAX_BUFFSIZE, 0);
    recv(client_socket, book.name, sizeof(book.name), 0);

    send(client_socket, "Enter number of copies: ", MAX_BUFFSIZE, 0);
    recv(client_socket, &book.copies, sizeof(book.copies), 0);

    book.delete = 0;

    if (testing_mode) {
        // Open a temporary or mock test book file for testing
        bookFile = fopen(test_booksCol, "rb+"); // Temporary test file
    } else {
        bookFile = fopen(booksCol, "rb+");  // Regular file
    }

    if (bookFile == NULL) {
        send(client_socket, "Error opening book file.\n", MAX_BUFFSIZE, 0);
        return;
    }

    fd = fileno(bookFile);
    lock_file(fd, F_WRLCK); // Acquire write lock
    struct Book bk;
    int book_added = 0;
    while (fread(&bk, sizeof(struct Book), 1, bookFile)) {
        if (bk.delete == 1) {
            fseek(bookFile, -sizeof(struct Book), SEEK_CUR);
            fwrite(&book, sizeof(struct Book), 1, bookFile);
            book_added = 1;
            break;
        }
    }
    if (!book_added) {
        fseek(bookFile, 0, SEEK_END);
        fwrite(&book, sizeof(struct Book), 1, bookFile);
    }

    lock_file(fd, F_UNLCK); // Release lock
    fclose(bookFile);

    send(client_socket, "Book added successfully.\n", MAX_BUFFSIZE, 0);
    return;
}

void updateBookCopies(int client_socket, int testing_mode) {
    char class_id[4] = {0};
    int new_copies;
    struct Book book;
    FILE *bookFile;
    int fd;
    char buffer[MAX_BUFFSIZE] = {0};

    send(client_socket, "Enter class ID of the book to update: ", MAX_BUFFSIZE, 0);
    recv(client_socket, class_id, sizeof(class_id), 0);

    send(client_socket, "Enter the new number of copies: ", MAX_BUFFSIZE, 0);
    recv(client_socket, &new_copies, sizeof(new_copies), 0);

    if (testing_mode) {
        // Open a temporary or mock test book file for testing
        bookFile = fopen(test_booksCol, "rb+"); // Temporary test file
    } else {
        bookFile = fopen(booksCol, "rb+");  // Regular file
    }

    if (bookFile == NULL) {
        send(client_socket, "Error opening book file.\n", MAX_BUFFSIZE, 0);
        return;
    }

    fd = fileno(bookFile);
    lock_file(fd, F_WRLCK); // Acquire write lock

    fseek(bookFile, 0, SEEK_SET);
    int found = 0;
    while (fread(&book, sizeof(struct Book), 1, bookFile)) {
        if (book.delete == 0 && strcmp(book.class_id, class_id) == 0) {
            found = 1;
            book.copies = new_copies; // Set (not add) the new copies count
            fseek(bookFile, -sizeof(struct Book), SEEK_CUR);
            fwrite(&book, sizeof(struct Book), 1, bookFile);
            break;
        }
    }

    lock_file(fd, F_UNLCK); // Release lock
    fclose(bookFile);

    if (found) {
        send(client_socket, "Book copies updated successfully.\n", MAX_BUFFSIZE, 0);
    } else {
        send(client_socket, "Book not found.\n", MAX_BUFFSIZE, 0);
    }
}

void deleteBook(int client_socket, int testing_mode) {
    char class_id[4] = {0};
    struct Book book;
    FILE *bookFile;
    int fd;
    char buffer[MAX_BUFFSIZE] = {0};

    send(client_socket, "Enter class ID of the book to delete: ", MAX_BUFFSIZE, 0);
    recv(client_socket, class_id, sizeof(class_id), 0);

    if (testing_mode) {
        // Open a temporary or mock test book file for testing
        bookFile = fopen(test_booksCol, "rb+"); // Temporary test file
    } else {
        bookFile = fopen(booksCol, "rb+");  // Regular file
    }

    if (bookFile == NULL) {
        send(client_socket, "Error opening book file.\n", MAX_BUFFSIZE, 0);
        return;
    }

    fd = fileno(bookFile);
    lock_file(fd, F_WRLCK); // Acquire write lock
    fseek(bookFile, 0, SEEK_SET);

    int found = 0;
    while (fread(&book, sizeof(struct Book), 1, bookFile)) {
        if (strcmp(book.class_id, class_id) == 0 && book.delete == 0) {
            found = 1;
            book.delete = 1;
            fseek(bookFile, -sizeof(struct Book), SEEK_CUR);
            fwrite(&book, sizeof(struct Book), 1, bookFile);
            break;
        }
    }

    lock_file(fd, F_UNLCK); // Release lock
    fclose(bookFile);

    if (found) {
        send(client_socket, "Book deleted successfully.\n", MAX_BUFFSIZE, 0);
    } else {
        send(client_socket, "Book not found.\n", MAX_BUFFSIZE, 0);
    }
}

void allocateBook(int client_socket, int testing_mode) {
    char member_username[MAX_USERNAME_LENGTH];
    char book_class_id[4];
    int duration_days;
    struct Book book;
    struct Allocation allocation;
    FILE *allocFile;
    FILE *bookFile;
    int fd;

    // Receive member username
    send(client_socket, "Enter member username: ", MAX_BUFFSIZE, 0);
    recv(client_socket, member_username, sizeof(member_username), 0);

    // Receive book class ID
    send(client_socket, "Enter book class ID: ", MAX_BUFFSIZE, 0);
    recv(client_socket, book_class_id, sizeof(book_class_id), 0);

    // Receive allocation duration in days
    send(client_socket, "Enter allocation duration (days): ", MAX_BUFFSIZE, 0);
    recv(client_socket, &duration_days, sizeof(duration_days), 0);

    // Open the members file to check if the user exists
    FILE *membersFile = (testing_mode) ? fopen(test_memAccs, "rb") : fopen(memAccs, "rb");
    if (membersFile == NULL) {
        send(client_socket, "Error opening members file.\n", MAX_BUFFSIZE, 0);
        return;
    }

    struct Account member;
    int user_found = 0;
    while (fread(&member, sizeof(struct Account), 1, membersFile)) {
        if (strcmp(member.username, member_username) == 0) {
            user_found = 1;
            break;
        }
    }
    fclose(membersFile);

    if (!user_found) {
        send(client_socket, "User not found.\n", MAX_BUFFSIZE, 0);
        return;
    }

    // Open the books file to check if the book is available
    bookFile = (testing_mode) ? fopen(test_booksCol, "rb+") : fopen(booksCol, "rb+");
    if (bookFile == NULL) {
        send(client_socket, "Error opening books file.\n", MAX_BUFFSIZE, 0);
        return;
    }

    fd = fileno(bookFile);
    lock_file(fd, F_WRLCK); // Acquire write lock

    int book_found = 0;
    while (fread(&book, sizeof(struct Book), 1, bookFile)) {
        if (strcmp(book.class_id, book_class_id) == 0 && book.delete == 0) {
            book_found = 1;
            if (book.copies > 0) {
                book.copies--; // Decrease the number of available copies
                fseek(bookFile, -sizeof(struct Book), SEEK_CUR);
                fwrite(&book, sizeof(struct Book), 1, bookFile);
            } else {
                send(client_socket, "No copies available.\n", MAX_BUFFSIZE, 0);
                book_found = -1;
            }
            break;
        }
    }

    lock_file(fd, F_UNLCK); // Release lock
    fclose(bookFile);

    if (!book_found) {
        if (book_found == -1) {
            return; // No copies available
        }
        send(client_socket, "Book not found.\n", MAX_BUFFSIZE, 0);
        return;
    }

    // Proceed to add allocation record
    allocFile = (testing_mode) ? fopen(test_allocList, "rb+") : fopen(allocList, "rb+");
    if (allocFile == NULL) {
        send(client_socket, "Error opening allocation file.\n", MAX_BUFFSIZE, 0);
        return;
    }

    fd = fileno(allocFile);
    lock_file(fd, F_WRLCK); // Acquire write lock

    // Fill allocation details
    strcpy(allocation.name, member_username);
    strcpy(allocation.class_id, book_class_id);
    time_t tnow = time(NULL);
    strcpy(allocation.dateOfIssue, ctime(&tnow));

    struct tm *tm_info;
    time_t raw_time = tnow + (duration_days * 24 * 60 * 60);
    tm_info = localtime(&raw_time);
    raw_time = mktime(tm_info);
    strcpy(allocation.dateOfReturn, ctime(&raw_time));

    allocation.delete = 0;

    // Write to allocation file
    fseek(allocFile, 0, SEEK_SET);
    int alloc_write = 0;
    struct Allocation temp_alloc;
    while (fread(&temp_alloc, sizeof(struct Allocation), 1, allocFile)) {
        if (temp_alloc.delete == 1) {
            fseek(allocFile, -sizeof(struct Allocation), SEEK_CUR);
            fwrite(&allocation, sizeof(struct Allocation), 1, allocFile);
            alloc_write = 1;
            break;
        }
    }
    if (!alloc_write) {
        fseek(allocFile, 0, SEEK_END);
        fwrite(&allocation, sizeof(struct Allocation), 1, allocFile);
    }

    lock_file(fd, F_UNLCK); // Release lock
    fclose(allocFile);

    send(client_socket, "Book allocated successfully.\n", MAX_BUFFSIZE, 0);
}

void deallocateBook(int client_socket, int testing_mode) {
    char member_username[MAX_USERNAME_LENGTH];
    char book_class_id[4];
    struct Allocation allocation;
    FILE *allocFile;
    FILE *bookFile;
    int fd;

    send(client_socket, "Enter member username: ", MAX_BUFFSIZE, 0);
    recv(client_socket, member_username, sizeof(member_username), 0);

    send(client_socket, "Enter book class ID: ", MAX_BUFFSIZE, 0);
    recv(client_socket, book_class_id, sizeof(book_class_id), 0);

    allocFile = (testing_mode) ? fopen(test_allocList, "rb+") : fopen(allocList, "rb+");
    if (allocFile == NULL) {
        send(client_socket, "Error opening allocation file.\n", MAX_BUFFSIZE, 0);
        return;
    }

    fd = fileno(allocFile);
    lock_file(fd, F_WRLCK); // Acquire write lock

    int allocation_found = 0;
    fseek(allocFile, 0, SEEK_SET);
    while (fread(&allocation, sizeof(struct Allocation), 1, allocFile)) {
        if (allocation.delete == 0 && strcmp(allocation.name, member_username) == 0 && strcmp(allocation.class_id, book_class_id) == 0) {
            allocation_found = 1;
            allocation.delete = 1; // Mark allocation as deleted
            fseek(allocFile, -sizeof(struct Allocation), SEEK_CUR);
            fwrite(&allocation, sizeof(struct Allocation), 1, allocFile);
            break;
        }
    }

    lock_file(fd, F_UNLCK); // Release lock
    fclose(allocFile);

    if (!allocation_found) {
        send(client_socket, "Allocation not found.\n", MAX_BUFFSIZE, 0);
        return;
    }

    // Increment book copies
    bookFile = (testing_mode) ? fopen(test_booksCol, "rb+") : fopen(booksCol, "rb+");
    if (bookFile == NULL) {
        send(client_socket, "Error opening book file.\n", MAX_BUFFSIZE, 0);
        return;
    }

    fd = fileno(bookFile);
    lock_file(fd, F_WRLCK); // Acquire write lock
    fseek(bookFile, 0, SEEK_SET);

    struct Book book;
    int book_updated = 0;
    while (fread(&book, sizeof(struct Book), 1, bookFile)) {
        if (strcmp(book.class_id, book_class_id) == 0 && book.delete == 0) {
            book.copies++;
            fseek(bookFile, -sizeof(struct Book), SEEK_CUR);
            fwrite(&book, sizeof(struct Book), 1, bookFile);
            book_updated = 1;
            break;
        }
    }

    lock_file(fd, F_UNLCK); // Release lock
    fclose(bookFile);

    if (book_updated) {
        send(client_socket, "Book deallocated successfully.\n", MAX_BUFFSIZE, 0);
    } else {
        send(client_socket, "Book not found for updating.\n", MAX_BUFFSIZE, 0);
    }
}

void seeAllocationsForUser(int client_socket, int testing_mode) {
    char buffer[MAX_BUFFSIZE] = {0};
    char username[MAX_USERNAME_LENGTH];

    send(client_socket, "Enter the username: ", MAX_BUFFSIZE, 0);
    recv(client_socket, username, sizeof(username), 0);

    FILE *allocFile = (testing_mode) ? fopen(test_allocList, "rb+") : fopen(allocList, "rb+");
    if (allocFile == NULL) {
        send(client_socket, "Error opening allocation file.\n", MAX_BUFFSIZE, 0);
        return;
    }

    int fd = fileno(allocFile);
    lock_file(fd, F_RDLCK); // Acquire read lock
    fseek(allocFile, 0, SEEK_SET);

    strcpy(buffer, "Allocations for user:\n");
    struct Allocation allocation;
    while (fread(&allocation, sizeof(struct Allocation), 1, allocFile)) {
        if (allocation.delete == 0 && strcmp(allocation.name, username) == 0) {
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

void viewAllUsers(int client_socket, int testing_mode) {
    char buffer[MAX_BUFFSIZE] = {0};
    FILE *memFile = (testing_mode) ? fopen(test_memAccs, "rb+") : fopen(memAccs, "rb+");
    if (memFile == NULL) {
        send(client_socket, "Error opening member file.\n", MAX_BUFFSIZE, 0);
        return;
    }

    int fd = fileno(memFile);
    lock_file(fd, F_RDLCK); // Acquire read lock
    fseek(memFile, 0, SEEK_SET);

    strcpy(buffer, "All users/members:\n");
    struct Account member;

    while (fread(&member, sizeof(struct Account), 1, memFile)) {
        char temp[100];
        sprintf(temp, "Username: %s, Joining Time: %s\n", member.username, member.joiningTime);
        strcat(buffer, temp);
    }

    lock_file(fd, F_UNLCK); // Release lock
    fclose(memFile);
    send(client_socket, buffer, MAX_BUFFSIZE, 0);
}

void exitAdminPanel(int client_socket) {
    send(client_socket, "Exiting admin panel.\n", MAX_BUFFSIZE, 0);
}
