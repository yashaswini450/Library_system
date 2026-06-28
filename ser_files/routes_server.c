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

int mock_channel_1[2]; // client to server write
int mock_channel_0[2]; // server to client write

/*
 sockfd = 1 --> client in focus. client writes, client reads
 sockfd = 0 --> server in focus. server writes, server reads

 my_send(sockfd=0,...) --> server writes to client
 my_recv(sockfd=0,...) --> server reads from client
*/

void activate_mock_channel() {
    pipe(mock_channel_0);
    pipe(mock_channel_1);
}

ssize_t my_send(int sockfd, const void *buf, size_t len, int flags) {
    // Write to the pipe
    ssize_t bytes_written = write((sockfd == 0) ? mock_channel_0[1] : mock_channel_1[1], buf, len);
    if (bytes_written == -1) {
        perror("Write to pipe failed");
        return -1;
    }
    return bytes_written;
}

ssize_t my_recv(int sockfd, void *buf, size_t len, int flags) {
    // Read from the pipe
    ssize_t bytes_read = read((sockfd == 1) ? mock_channel_0[0] : mock_channel_1[0], buf, len);
    if (bytes_read == -1) {
        perror("Read from pipe failed");
        return -1;
    }
    return bytes_read;
}

void deactivate_mock_channel() {
    close(mock_channel_0[0]);
    close(mock_channel_0[1]);
    close(mock_channel_1[0]);
    close(mock_channel_1[1]);
}

void handle_connection(int client_socket, struct sockaddr_in *address) {
    struct Account account;

    int r = entry(client_socket);

    if (r == END_CONN) {
        printf("Bad input.\n");
        goto end_conn;
    } else if (r == TO_REGISTER) {
        registerAccount(client_socket);
    } else {
        r = login(client_socket, &account);
        if (r == TO_ADMIN) admin(client_socket, &account);
        else if (r == TO_MEMBER) member(client_socket, &account);
    }

end_conn:
    close(client_socket);
    printf("Client %s:%d exits.\n", inet_ntoa((*address).sin_addr), ntohs((*address).sin_port));
}

void admin(int client_socket, struct Account *acc) {
    // Admin operations
    char buffer[MAX_BUFFSIZE] = {0};
    char x[100];
    sprintf(x, "Welcome admin \"%s\"", acc->username);
    strcpy(buffer, formatHeading(x));
    strcat(buffer, "\n\n1. See all books and available copies.\n2. See incumbent allocations.\n3. Add book.\n4. Update copies of a particular book.\n5. Delete book (all copies).\n6. Allocate a book.\n7. Deallocate a book.\n8. See allocations to a particular user.\n9. View all users/members.\n10. Exit\n");
    send(client_socket, buffer, MAX_BUFFSIZE, 0);

    while (1) {
        memset(buffer, '\0', sizeof(buffer));
        send(client_socket, "\nEnter choice:", MAX_BUFFSIZE, 0);

        int choice;
        recv(client_socket, &choice, sizeof(choice), 0);
        printf("Recvd choice %d\n", choice);

        switch (choice) {
            case 1:
                seeAllBooks(client_socket, 0);
                break;

            case 2:
                seeAllocations(client_socket, 0);
                break;

            case 3:
                addBook(client_socket, 0);
                break;

            case 4:
                updateBookCopies(client_socket, 0);
                break;

            case 5:
                deleteBook(client_socket, 0);
                break;

            case 6:
                allocateBook(client_socket, 0);
                break;

            case 7:
                deallocateBook(client_socket, 0);
                break;

            case 8:
                seeAllocationsForUser(client_socket, 0);
                break;

            case 9:
                viewAllUsers(client_socket, 0);
                break;

            case 10:
                send(client_socket, "Exiting admin panel.\n", MAX_BUFFSIZE, 0);
                return;

            default:
                send(client_socket, "Invalid choice.\n", MAX_BUFFSIZE, 0);
                break;
        }
    }
}

void member(int client_socket, struct Account *acc) {
    // Member operations
    char buffer[MAX_BUFFSIZE] = {0};
    char x[100];
    sprintf(x, "Welcome member \"%s\"", acc->username);
    strcpy(buffer, formatHeading(x));
    strcat(buffer, "\n\n1. View books in library.\n2. View current issues (allocations).\n3. Exit\n");
    send(client_socket, buffer, MAX_BUFFSIZE, 0);

    while (1) {
        memset(buffer, '\0', sizeof(buffer));
        send(client_socket, "\nEnter choice: ", MAX_BUFFSIZE, 0);

        int choice;
        recv(client_socket, &choice, sizeof(choice), 0);

        switch (choice) {
            case 1:
                viewBooksInLibrary(client_socket, 0);
                break;

            case 2:
                viewCurrentIssues(client_socket, acc, 0);
                break;

            case 3:
                exitMemberPanel(client_socket);
                return;

            default:
                send(client_socket, "Invalid choice.\n", strlen("Invalid choice.\n"), 0);
                break;
        }
    }
}

int checkCredentials(const char *username, const char *password, enum AccountType type, struct Account *account) {
    FILE *file;
    int fd;
    if (type == MEMBER)
        file = fopen(memAccs, "rb");
    else if (type == ADMIN)
        file = fopen(admAccs, "rb");
    else {
        printf("Invalid account type.\n");
        return INVALID_ENTITY;
    }

    if (file == NULL) {
        printf("Error opening file.\n");
        return INVALID_ENTITY;
    }

    fd = fileno(file);
    lock_file(fd, F_RDLCK); // Acquire a read lock

    fseek(file, 0, SEEK_SET);

    while (fread(account, sizeof(struct Account), 1, file)) {
        if (strcmp((*account).username, username) == 0 &&
            strcmp((*account).password, password) == 0 &&
            (*account).type == type) {
            lock_file(fd, F_UNLCK); // Release the lock
            fclose(file);
            return ACCOUNT_FOUND;
        }
    }

    lock_file(fd, F_UNLCK); // Release the lock
    fclose(file);
    return INVALID_CREDENTIALS;
}

void registerAccount(int client_socket) {

    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int accountType;

    send(client_socket, "Enter username: ", strlen("Enter username: "), 0);
    recv(client_socket, username, MAX_USERNAME_LENGTH, 0);
    username[strcspn(username, "\n")] = '\0';

    send(client_socket, "Enter password: ", strlen("Enter password: "), 0);
    recv(client_socket, password, MAX_PASSWORD_LENGTH, 0);
    password[strcspn(password, "\n")] = '\0';

    send(client_socket, "Enter account type (0 for member, 1 for admin): ", strlen("Enter account type (0 for member, 1 for admin): "), 0);
    char accountTypeStr[2];
    recv(client_socket, accountTypeStr, 2, 0);
    accountType = atoi(accountTypeStr);

    FILE *file;
    int fd;
    if (accountType == 0) {
        file = fopen(memAccs, "rb+");
    } else if (accountType == 1) {
        file = fopen(admAccs, "rb+");
    } else {
        send(client_socket, "Invalid account type.\n", strlen("Invalid account type.\n"), 0);
        return;
    }

    if (file == NULL) {
        send(client_socket, "Error opening file.\n", strlen("Error opening file.\n"), 0);
        return;
    }

    fd = fileno(file);
    lock_file(fd, F_WRLCK); // Acquire a write lock

    // Check for duplicate username before adding
    struct Account existing;
    fseek(file, 0, SEEK_SET);
    while (fread(&existing, sizeof(struct Account), 1, file)) {
        if (strcmp(existing.username, username) == 0) {
            lock_file(fd, F_UNLCK);
            fclose(file);
            send(client_socket, "Username already exists. Please choose a different username.\n",
                 strlen("Username already exists. Please choose a different username.\n"), 0);
            return;
        }
    }

    struct Account newAccount;
    strcpy(newAccount.username, username);
    strcpy(newAccount.password, password);
    newAccount.type = (accountType == 1) ? ADMIN : MEMBER;
    time_t t = time(NULL);
    strcpy(newAccount.joiningTime, ctime(&t));

    fseek(file, 0, SEEK_END);
    fwrite(&newAccount, sizeof(struct Account), 1, file);

    lock_file(fd, F_UNLCK); // Release the lock
    fclose(file);

    send(client_socket, "Account registered successfully. Open a new session to login.\n", strlen("Account registered successfully. Open a new session to login.\n"), 0);
}

int login(int client_socket, struct Account *account) {

    char usrnm[MAX_USERNAME_LENGTH];
    char passwd[MAX_PASSWORD_LENGTH];
    enum AccountType type;

    send(client_socket, "Enter username: ", MAX_USERNAME_LENGTH, 0);
    int valread = recv(client_socket, usrnm, MAX_USERNAME_LENGTH, 0);
    if (valread <= 0) {
        printf("Client disconnected.\n");
        return END_CONN;
    }
    printf("Received: %s\n", usrnm);

    send(client_socket, "Enter password: ", MAX_PASSWORD_LENGTH, 0);
    valread = recv(client_socket, passwd, MAX_PASSWORD_LENGTH, 0);
    if (valread <= 0) {
        printf("Client disconnected.\n");
        return END_CONN;
    }
    printf("Received: %s\n", passwd);

    send(client_socket, "Login as (0: Member, 1: Admin): ", 50, 0);
    char buff[2] = {0};
    valread = recv(client_socket, buff, 2, 0);
    if (valread <= 0) {
        printf("Client disconnected.\n");
        return END_CONN;
    }
    printf("Received: %s\n", buff);

    if (strcmp(buff, "0") == 0) type = MEMBER;
    else if (strcmp(buff, "1") == 0) type = ADMIN;
    else {
        printf("Invalid account type.\n");
        return END_CONN;
    }

    int r = checkCredentials(usrnm, passwd, type, account);
    if (r == ACCOUNT_FOUND) {
        printf("Account found.\n");
        (type == 1) ? send(client_socket, "Account found. Logging in as Admin...\n", MAX_BUFFSIZE, 0) :
                      send(client_socket, "Account found. Logging in as Member...\n", MAX_BUFFSIZE, 0);
        return (type == 0) ? TO_MEMBER : TO_ADMIN;
    } else {
        printf("Invalid credentials.\n");
        send(client_socket, "Invalid credentials.\n", strlen("Invalid credentials.\n"), 0);
    }
    return END_CONN;
}

int entry(int client_socket) {
    char *msg = "1. Register account.\n2. Login.\n\nEnter choice: ";
    char choice[2];

    send(client_socket, msg, strlen(msg), 0);
    if (recv(client_socket, choice, 2, 0) <= 0) {
        printf("Client disconnected.\n");
        return END_CONN;
    }

    if (strcmp(choice, "1") == 0) {
        return TO_REGISTER;
    } else if (strcmp(choice, "2") == 0) {
        return TO_LOGIN;
    } else {
        printf("Invalid choice.\n");
        return END_CONN;
    }
}
