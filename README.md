# Library_system: Online Library Management System

An **Online Library Management System (OLMS)** built with a multi-threaded concurrent server architecture using C socket programming and system calls.

## Abstract

This project designs and develops an Online Library Management System that provides basic library functionalities while ensuring data security and concurrency control. The system employs socket programming to enable multiple clients to access the library database concurrently. System calls are utilized extensively for process management, file handling, file locking, multithreading, and interprocess communication.

## Features

- **User Authentication**: Members pass through a login system to access their accounts, ensuring data privacy and security.
- **Administrative Access**: Password-protected administrative access for librarians to manage book transactions and member information.
- **Book Management**: Admins can add, delete, modify, and search for specific book details.
- **File-Locking Mechanisms**: Implemented using `fcntl` system calls to protect critical data sections and ensure mutual exclusion (concurrency control).
- **Concurrent Access**: Uses `fork()`-based concurrent server to service multiple clients simultaneously via socket programming.

## Project Structure

```
Library_system/
├── auth.h              # Shared header: data structures, macros, prototypes
├── auth.c              # Utility functions: file locking, time formatting, heading
├── makefile            # Build configuration
├── cli_files/
│   ├── cli.h           # Client function prototypes
│   ├── cli.c           # Client main: socket creation, connection, routing
│   └── routes_client.c # Client route handlers (admin/member/register/login)
├── ser_files/
│   ├── ser.h           # Server function prototypes
│   ├── ser.c           # Server main: socket, bind, listen, fork per client
│   ├── routes_server.c # Server routing, authentication, mock channel
│   ├── mem.c           # Member operations (view books, view issues)
│   └── adm.c           # Admin operations (CRUD books, allocations, users)
└── db_files/
    ├── memAccounts.bin     # Member account records
    ├── adminAccounts.bin   # Admin account records
    ├── booksCollection.bin # Book records
    └── allocationsList.bin # Book allocation records
```

## How to Run

### Prerequisites
- Linux or macOS with GCC installed
- The project uses Linux-specific system calls (`fcntl`, `fork`, `socket`)

### Build

```bash
make all
```

This generates two executables: `server` and `client`.

### Start the Server

```bash
./server
```

The server starts listening on `127.0.0.1:8080`.

### Connect a Client

Open a new terminal and run:

```bash
./client
```

You can open multiple client terminals simultaneously — the server handles each in a separate process.

### Clean Build Artifacts

```bash
make clean
```

## Usage

### Registration

1. Run `./client`
2. Select option `1` (Register account)
3. Enter username, password, and account type (`0` for member, `1` for admin)
4. Open a new session to log in

### Member Operations

After logging in as a member:
- **1**: View all books in the library with available copies
- **2**: View your currently issued books and due dates
- **3**: Exit

### Admin Operations

After logging in as an admin:
- **1**: See all books and available copies
- **2**: See all current allocations
- **3**: Add a new book (class ID, name, copies)
- **4**: Update copies of a book
- **5**: Delete a book
- **6**: Allocate a book to a member (specify username, book class ID, duration in days)
- **7**: Deallocate a book from a member
- **8**: View allocations for a specific user
- **9**: View all registered members
- **10**: Exit

## Implementation Details

| Feature | Mechanism |
|---|---|
| Concurrent clients | `fork()` — each client gets its own process |
| File locking | `fcntl` with `F_RDLCK` / `F_WRLCK` / `F_UNLCK` |
| Data storage | Binary files (`.bin`) with fixed-size structs |
| Communication | TCP sockets on port 8080 |
| Soft delete | `delete` flag on Book and Allocation structs |
| Auth | Username + password + account type checked against binary DB |

## Data Structures

```c
struct Account {
    char username[50];
    char password[50];
    enum AccountType type;  // MEMBER or ADMIN
    char joiningTime[30];
};

struct Book {
    char class_id[4];
    char name[50];
    int copies;
    uint8_t delete;  // soft delete flag
};

struct Allocation {
    char class_id[4];
    char name[50];       // member username
    char dateOfIssue[30];
    char dateOfReturn[30];
    uint8_t delete;
};
```

## Notes

- The `db_files/` directory must exist before running the server. Empty binary files are included to bootstrap the database.
- The project is designed for Linux. On macOS, `SO_REUSEPORT` may behave differently; if you encounter issues, remove it from `setsockopt`.
- Binary files store fixed-size structs directly — no external database is needed.
