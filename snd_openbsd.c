#include <stdlib.h>
#include <stdio.h>
#include <sndio.h>

#include "misc.h"

struct sio_hdl *snd_handle;
off_t frame;

void
snd_callback(void *arg, int delta) {
	frame += delta;
	printf("\rplaying frame %lld", frame);
}

int
snd_open(void) {
	struct sio_par p;

	frame = 0;
	
	snd_handle = sio_open(NULL, SIO_PLAY, 0);
	sio_onmove(snd_handle, &snd_callback, NULL);
	sio_initpar(&p);

	p.bits = SIZE;
	p.pchan = CHANNELS;
	p.rate = RATE;
	p.xrun = SIO_SYNC;

	sio_setpar(snd_handle, &p);
	sio_getpar(snd_handle, &p);

	if (p.bits != SIZE) {
		fprintf(stderr, "failed to set sample size: %d != %d\n", p.bits, SIZE);
		return 0;
	}

	if (p.pchan != CHANNELS) {
		fprintf(stderr, "failed to set number of channels: %d != %d\n", p.pchan, CHANNELS);
		return 0;
	}

	if (p.rate != RATE) {
		fprintf(stderr, "failed to set sample size: %d != %d\n", p.rate, RATE);
		return 0;
	}

	sio_start(snd_handle);
	return 1;
}

int
snd_write(unsigned char *data, int len) {
	return sio_write(snd_handle, data, len);
}
