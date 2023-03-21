CC = gcc

all: primes1 primes2 test

primes1: prime1.c
	$(CC) prime1.c -o primes1

primes2: prime2.c
	$(CC) prime2.c -o primes2 -lm

test: test.c sort.c
	$(CC) -o test test.c sort.c

clean:
	rm -f primes1 primes2 test