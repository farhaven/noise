#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "misc.h"

int soundfd;

int
snd_write(unsigned char *data, int len) {
	return (int) write(soundfd, data, len);
}

int
snd_open(void) {
	int arg, status;

	soundfd = open("/dev/dsp", O_RDWR);
	if (soundfd < 0) {
		perror("could not open /dev/dsp");
		exit(1);
	}

    arg = SIZE;
    status = ioctl(soundfd, SOUND_PCM_WRITE_BITS, &arg);
    if (status == -1) perror("SOUND_PCM_WRITE_BITS ioctl failed");
    if (arg != SIZE) perror("unable to set sample size");

    arg = CHANNELS;
    status = ioctl(soundfd, SOUND_PCM_WRITE_CHANNELS, &arg);
    if (status == -1) perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
    if (arg != CHANNELS) perror("unable to set number of channels");

    arg = RATE;
    status = ioctl(soundfd, SOUND_PCM_WRITE_RATE, &arg);
    if (status == -1) perror("SOUND_PCM_WRITE_WRITE ioctl failed");

	return soundfd;
}
