CFLAGS = -Wall -std=c99
CHKFLAGS = --enable=all --inconclusive --std=posix

all: server client

server: server.c
	gcc $(CFLAGS) server.c -o server

client: client.c
	gcc $(CFLAGS) -pthread client.c -o client

sanitized: sanitized-server sanitized-client

sanitized-server:
	gcc $(CFLAGS) -fsanitize=address server.c -o server

sanitized-client:
	gcc $(CFLAGS) -fsanitize=address client.c -o client

check: check-server check-client

check-server:
	cppcheck $(CHKFLAGS) server.c
	./checkpatch.pl --no-tree -f server.c

check-client:
	cppcheck $(CHKFLAGS) client.c
	./checkpatch.pl --no-tree -f client.c

clean:
	rm server client
