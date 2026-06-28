#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <termios.h>
#include "../auth.h"
#include "cli.h"

// Reads a password without echoing characters to the terminal
static void readPassword(char *password, int maxLen) {
    struct termios old_term, new_term;
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(tcflag_t)ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
    scanf("%s", password);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    printf("\n"); // move to next line after hidden input
}


// Function to handle admin client interactions
void admin_client(int client_socket) {
    char buffer[MAX_BUFFSIZE] = {0};
    int choice;

    recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive initial server message
    printf("%s", buffer);
    memset(buffer, '\0', sizeof(buffer));

    while (1) {
        memset(buffer, '\0', sizeof(buffer));
        recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive menu options
        printf("%s", buffer);
        memset(buffer, '\0', sizeof(buffer));

        // Prompt and wait for user input
        scanf("%d", &choice);

        // Send the choice to the server
        send(client_socket, &choice, sizeof(choice), 0);

        switch (choice) {
            case 1:
            case 2:
                memset(buffer, '\0', sizeof(buffer));
                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive server response
                printf("%s", buffer);
                break;

            case 3: {
                char class_id[4] = {0}, name[50] = {0};
                int copies;

                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for class ID
                printf("%s", buffer);
                memset(buffer, '\0', sizeof(buffer));

                scanf("%3s", class_id); // Input class ID (max 3 chars + null)
                send(client_socket, class_id, sizeof(class_id), 0);

                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for name
                printf("%s", buffer);
                memset(buffer, '\0', sizeof(buffer));

                // Consume leftover newline before reading full line
                int c; while ((c = getchar()) != '\n' && c != EOF);
                scanf(" %49[^\n]", name); // Input full book name (supports spaces)
                send(client_socket, name, sizeof(name), 0);

                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for copies
                printf("%s", buffer);
                memset(buffer, '\0', sizeof(buffer));

                scanf("%d", &copies); // Input copies
                send(client_socket, &copies, sizeof(copies), 0);

                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive server response
                printf("%s", buffer);
                break;
            }

            case 4: {
                char class_id[4];
                int new_copies;
                memset(buffer, '\0', sizeof(buffer));
                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for class ID
                printf("%s", buffer);
                memset(buffer, '\0', sizeof(buffer));

                scanf("%s", class_id); // Input class ID
                send(client_socket, class_id, sizeof(class_id), 0);

                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for new copies
                printf("%s", buffer);
                memset(buffer, '\0', sizeof(buffer));

                scanf("%d", &new_copies); // Input new copies
                send(client_socket, &new_copies, sizeof(new_copies), 0);

                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive server response
                printf("%s", buffer);
                break;
            }

            case 5: {
                char class_id[4];

                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for class ID
                printf("%s", buffer);
                memset(buffer, '\0', sizeof(buffer));

                scanf("%s", class_id); // Input class ID
                send(client_socket, class_id, sizeof(class_id), 0);

                memset(buffer, '\0', sizeof(buffer));
                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive server response
                printf("%s", buffer);
                break;
            }

            case 6: {
                char member_username[MAX_USERNAME_LENGTH], book_class_id[4];
                int duration_days;

                // Receive prompts from the server and input member username
                recv(client_socket, buffer, MAX_BUFFSIZE, 0);
                printf("%s", buffer);
                memset(buffer, '\0', sizeof(buffer));
                scanf("%s", member_username);
                send(client_socket, member_username, sizeof(member_username), 0);

                // Receive prompts from the server and input book class ID
                recv(client_socket, buffer, MAX_BUFFSIZE, 0);
                printf("%s", buffer);
                memset(buffer, '\0', sizeof(buffer));
                scanf("%s", book_class_id);
                send(client_socket, book_class_id, sizeof(book_class_id), 0);

                // Receive prompts from the server and input allocation duration
                recv(client_socket, buffer, MAX_BUFFSIZE, 0);
                printf("%s", buffer);
                memset(buffer, '\0', sizeof(buffer));
                scanf("%d", &duration_days);
                send(client_socket, &duration_days, sizeof(duration_days), 0);

                // Receive response from the server
                recv(client_socket, buffer, MAX_BUFFSIZE, 0);
                printf("%s", buffer);
                break;
            }

            case 7: {
                char member_username[MAX_USERNAME_LENGTH], book_class_id[4];

                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for member username
                printf("%s", buffer);
                memset(buffer, '\0', sizeof(buffer));

                scanf("%s", member_username); // Input member username
                send(client_socket, member_username, sizeof(member_username), 0);

                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for book class ID
                printf("%s", buffer);
                memset(buffer, '\0', sizeof(buffer));

                scanf("%s", book_class_id); // Input book class ID
                send(client_socket, book_class_id, sizeof(book_class_id), 0);

                memset(buffer, '\0', sizeof(buffer));
                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive server response
                printf("%s", buffer);
                break;
            }

            case 8: {
                char member_username[MAX_USERNAME_LENGTH];

                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for member username
                printf("%s", buffer);
                memset(buffer, '\0', sizeof(buffer));

                scanf("%s", member_username); // Input member username
                send(client_socket, member_username, sizeof(member_username), 0);

                memset(buffer, '\0', sizeof(buffer));
                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive server response
                printf("%s", buffer);
                memset(buffer, '\0', sizeof(buffer));

                break;
            }

            case 9: {
                memset(buffer, '\0', sizeof(buffer));
                recv(client_socket, buffer, sizeof(buffer), 0); // Receive and print all members info
                printf("%s\n", buffer);
                break;
            }

            case 10:
                printf("Exiting admin panel.\n");
                return;

            default:
                printf("Invalid choice.\n");
                break;
        }
    }
}

// Function to handle member client interactions
void member_client(int client_socket) {
    char buffer[MAX_BUFFSIZE] = {0};
    int choice;

    // Receive the initial welcome message from the server
    recv(client_socket, buffer, MAX_BUFFSIZE, 0);
    printf("%s", buffer);
    memset(buffer, '\0', sizeof(buffer));

    while (1) {
        recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive menu options
        printf("%s", buffer);

        scanf("%d", &choice);
        // Send the choice to the server
        send(client_socket, &choice, sizeof(choice), 0);

        switch (choice) {
            case 1: // View books in library
                memset(buffer, 0, MAX_BUFFSIZE);
                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive books info
                printf("%s", buffer);
                break;

            case 2: // View current issues (allocations)
                memset(buffer, 0, MAX_BUFFSIZE);
                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive issues info
                printf("%s", buffer);
                break;

            case 3: // Exit
                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive exit message
                printf("%s", buffer);
                return;

            default:
                recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive invalid choice message
                printf("%s", buffer);
                break;
        }
    }
}

// Function to handle account registration
void registerAccount_client(int client_socket) {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    char accountType[2];
    char buffer[MAX_BUFFSIZE];
    memset(buffer, '\0', sizeof(buffer));
    printf("%s\n", formatHeading("<< REGISTER >>"));

    recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for username
    printf("%s", buffer);
    memset(buffer, '\0', sizeof(buffer));
    scanf("%s", username);
    send(client_socket, username, MAX_USERNAME_LENGTH, 0);

    recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for password
    printf("%s", buffer);
    memset(buffer, '\0', sizeof(buffer));
    readPassword(password, MAX_PASSWORD_LENGTH); // Hidden input
    send(client_socket, password, MAX_PASSWORD_LENGTH, 0);

    recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for account type
    printf("%s", buffer);
    memset(buffer, '\0', sizeof(buffer));
    scanf("%s", accountType);
    send(client_socket, accountType, 2, 0);

    recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive server response
    printf("%s", buffer);
}

// Function to handle login and direct to admin or member client
int login_client(int client_socket) {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    char accountType[2];
    char buffer[MAX_BUFFSIZE];

    printf("%s\n", formatHeading("<< LOGIN >>"));

    recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for username
    printf("%s", buffer);
    memset(buffer, '\0', sizeof(buffer));
    scanf("%s", username);
    send(client_socket, username, MAX_USERNAME_LENGTH, 0);

    recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for password
    printf("%s", buffer);
    memset(buffer, '\0', sizeof(buffer));
    readPassword(password, MAX_PASSWORD_LENGTH); // Hidden input
    send(client_socket, password, MAX_PASSWORD_LENGTH, 0);

    recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Prompt for account type
    printf("%s", buffer);
    memset(buffer, '\0', sizeof(buffer));
    scanf("%s", accountType);
    send(client_socket, accountType, 2, 0);

    recv(client_socket, buffer, MAX_BUFFSIZE, 0); // Receive server response
    printf("%s", buffer);

    // Check if login was successful and if the user is an admin or a member
    if (strcmp(buffer, "Account found. Logging in as Admin...\n") == 0) {
        admin_client(client_socket); // Call the admin_client function
    } else if (strcmp(buffer, "Account found. Logging in as Member...\n") == 0) {
        member_client(client_socket); // Call the member_client function
    }
    return END_CONN;
}
