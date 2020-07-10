CC := clang
CFLAGS := -g -Wall -Werror -Wno-unused-function -Wno-unused-variable -I$(HOME)/.local/include
LDFLAGS := -L$(HOME)/.local/lib

all: server client test_client

clean:
	rm -rf server server.dSYM client client.dSYM

server: server.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o server server.c -lpthread -ljansson

client: client.c ui.c ui.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o client client.c -lform -lncurses -lpthread

read_file: read_file.c 
	$(CC) $(CFLAGS) $(LDFLAGS) -static -o read_file read_file.c -ljansson

test_client: testing_clients.c
	$(CC) $(CFLAGS) $(LDFLAGS) -static -o testing_clients testing_clients.c