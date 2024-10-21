CC = gcc
CFLAGS = -Wall

# Source files and corresponding executables
SERVER_SRC = server.c
SERVER_EXE = server

CLIENT_SRC = client.c
CLIENT_EXE = client

# Default target
all: $(SERVER_EXE) $(CLIENT_EXE)

# Compile the server
$(SERVER_EXE): $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_EXE)

# Compile the client
$(CLIENT_EXE): $(CLIENT_SRC)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_EXE)

# Clean up the executables
clean:
	rm -f $(SERVER_EXE) $(CLIENT_EXE)
