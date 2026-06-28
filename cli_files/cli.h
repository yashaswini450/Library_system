// Function to handle admin client interactions
void admin_client(int client_socket);
// Function to handle member client interactions
void member_client(int client_socket);
// Function to handle account registration
void registerAccount_client(int client_socket);
// Function to handle login and direct to admin or member client
int login_client(int client_socket);
