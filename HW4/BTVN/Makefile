CC = gcc
CFLAGS = -Wall -Wextra
SERVER = server
CLIENT = client
PORT = 5500
IP = 127.0.0.1

all: $(SERVER) $(CLIENT)

$(SERVER): server.c
	$(CC) $(CFLAGS) server.c -o $(SERVER)

$(CLIENT): client.c
	$(CC) $(CFLAGS) client.c -o $(CLIENT)

run-server: $(SERVER)
	./$(SERVER) $(PORT)

run-client: $(CLIENT)
	./$(CLIENT) $(IP) $(PORT)

clean:
	rm -f $(SERVER) $(CLIENT)
