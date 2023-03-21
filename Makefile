# Authors: Fatima Nadeem, Himanshi Lalwani
# OS Programming Assignment 2
# The makefile

CC = gcc

all: primes1 primes2 primes

primes1: prime1.c
	$(CC) prime1.c -o primes1

primes2: prime2.c
	$(CC) prime2.c -o primes2 -lm

primes: primes.c helper.c
	$(CC) -o primes primes.c helper.c

clean:
	rm -f primes1 primes2 primes