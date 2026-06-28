CC = gcc

CFLAGS = -Wall -w -Wextra -I.

# Source files
SRCS_SERVER = ser_files/ser.c ser_files/mem.c ser_files/adm.c ser_files/routes_server.c auth.c
SRCS_CLIENT = cli_files/cli.c cli_files/routes_client.c auth.c

# Header files
HEADERS_CLIENT = auth.h cli_files/cli.h
HEADERS_SERVER = auth.h ser_files/ser.h

# Output executables
TARGET_SERVER = server
TARGET_CLIENT = client


# rule to build everything
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# server executable
$(TARGET_SERVER): $(SRCS_SERVER) $(HEADERS_SERVER)
	$(CC) $(CFLAGS) -o $@ $(SRCS_SERVER)

# client executable
$(TARGET_CLIENT): $(SRCS_CLIENT) $(HEADERS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $(SRCS_CLIENT)

clean:
	rm -f $(TARGET_SERVER) $(TARGET_CLIENT)
