CC=clang
CCFLAGS=-Wall
LDFLAGS=-lm

noise: noise.o
	$(CC) $(LDFLAGS) -o noise noise.o

noise.o: noise.c
	$(CC) $(CCFLAGS) -c noise.c -o noise.o
