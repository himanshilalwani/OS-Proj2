CC = gcc
CFLAGS = -Wall -Werror -std=c99

all: test prime1 prime2

test: test.c
	$(CC) $(CFLAGS) test.c -o test

prime1: prime1.c
	$(CC) $(CFLAGS) prime1.c -o prime1

prime2: prime2.c
	$(CC) $(CFLAGS) prime2.c -o prime2

clean:
	rm -f test prime1 prime2
