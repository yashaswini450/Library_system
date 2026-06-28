extern int mock_channel_1[2]; // client to server write
extern int mock_channel_0[2]; // server to client write
void activate_mock_channel();
void deactivate_mock_channel();
ssize_t my_recv(int sockfd, void *buf, size_t len, int flags);
ssize_t my_send(int sockfd, const void *buf, size_t len, int flags);

void handle_connection(int client_socket, struct sockaddr_in *address);
int entry(int client_socket);
int login(int client_socket, struct Account * account);
void registerAccount(int client_socket);
int checkCredentials(const char *username, const char *password, enum AccountType type, struct Account * account);
void member(int client_socket, struct Account *acc);
void admin(int client_socket, struct Account *acc);

// Function prototypes for the cases for member
void viewBooksInLibrary(int client_socket, int testing_mode);
void viewCurrentIssues(int client_socket, struct Account *acc, int testing_mode);
void exitMemberPanel(int client_socket);

// Function prototypes for the cases for admin
void seeAllBooks(int client_socket, int testing_mode);
void seeAllocations(int client_socket, int testing_mode);
void addBook(int client_socket, int testing_mode);
void updateBookCopies(int client_socket, int testing_mode);
void deleteBook(int client_socket, int testing_mode);
void allocateBook(int client_socket, int testing_mode);
void deallocateBook(int client_socket, int testing_mode);
void seeAllocationsForUser(int client_socket, int testing_mode);
void viewAllUsers(int client_socket, int testing_mode);
void exitAdminPanel(int client_socket);
