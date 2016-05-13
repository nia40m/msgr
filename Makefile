CFLAGS = -Wall -std=c99
CHKFLAGS = --enable=all --inconclusive --std=posix

all: server client

server: server.c
	gcc $(CFLAGS) -pthread server.c -o server

client: client.o tui.o
	gcc $(CFLAGS) -o client -pthread client.o tui.o -lform -lncurses

client.o: client.c
	gcc $(CFLAGS) -c client.c

tui.o: tui.c
	gcc $(CFLAGS) -c tui.c

sanitized: sanitized-server sanitized-client

sanitized-server:
	gcc $(CFLAGS) -pthread -fsanitize=address server.c -o server

sanitized-client:
	gcc $(CFLAGS) -pthread -fsanitize=address client.c tui.c -o client -lform -lncurses

check: check-server check-client

check-server:
	cppcheck $(CHKFLAGS) server.c
	./checkpatch.pl --no-tree -f server.c

check-client:
	cppcheck $(CHKFLAGS) client.c
	./checkpatch.pl --no-tree -f client.c

clean:
	rm -f *.o
	rm -f server client
