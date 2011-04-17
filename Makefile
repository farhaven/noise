CC=clang
CCFLAGS=-Wall
LDFLAGS=-lm

noise: noise.o draw.o
	$(CC) $(LDFLAGS) -o noise noise.o draw.o

noise.o: noise.c draw.h
	$(CC) $(CCFLAGS) -c noise.c -o noise.o

draw.o: draw.c draw.h
	$(CC) $(CCFLAGS) -c draw.c -o draw.o
