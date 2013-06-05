CC=clang
CFLAGS=-Werror -g -pedantic -Wall -I/usr/local/include
LDFLAGS=-lm -lSDL -L/usr/local/lib

sdl: snd_sdl.o noise.o draw.o
	$(CC) $(LDFLAGS) -o noise noise.o draw.o snd_sdl.o

linux: snd_linux.o noise.o draw.o
	$(CC) $(LDFLAGS) -o noise noise.o draw.o snd_linux.o

openbsd: snd_openbsd.o noise.o draw.o
	$(CC) $(LDFLAGS) -lsndio -o noise noise.o draw.o snd_openbsd.o

noise.o: noise.c draw.h misc.h
	$(CC) $(CCFLAGS) -c noise.c -o noise.o

draw.o: draw.c draw.h
	$(CC) $(CCFLAGS) -c draw.c -o draw.o

snd_linux.o: snd_linux.c snd.h
	$(CC) $(CCFLAGS) -c snd_linux.c -o snd_linux.o

snd_openbsd.o: snd_openbsd.c snd.h
	$(CC) $(CCFLAGS) -c snd_openbsd.c -o snd_openbsd.o
