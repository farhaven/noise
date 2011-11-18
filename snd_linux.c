#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "misc.h"

int
snd_open(void) {
	int fd, arg;

	fd = open("/dev/dsp", O_RDWR);
	if (fd < 0) {
		perror("could not open /dev/dsp");
		exit(1);
	}

    arg = SIZE;
    status = ioctl(fd, SOUND_PCM_WRITE_BITS, &arg);
    if (status == -1) perror("SOUND_PCM_WRITE_BITS ioctl failed");
    if (arg != SIZE) perror("unable to set sample size");

    arg = CHANNELS;
    status = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &arg);
    if (status == -1) perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
    if (arg != CHANNELS) perror("unable to set number of channels");

    arg = RATE;
    status = ioctl(fd, SOUND_PCM_WRITE_RATE, &arg);
    if (status == -1) perror("SOUND_PCM_WRITE_WRITE ioctl failed");

	return fd;
}
