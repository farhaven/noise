#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/soundcard.h>

#include <math.h>

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#define MAX(x, y) ((x > y) ? x : y)
#define ABS(x) ((x >= 0) ? x : -x)

#define SIZE 8     /* sample size: 8 or 16 bits */
#define CHANNELS 2 /* 1 = mono 2 = stereo */
#define RATE (1000 * 1000) /* the sampling rate */

#define BUFLEN (1024 * 4)

void
circle(unsigned char *s, size_t len, float r, float x, float y) {
    size_t len_sample = len / 2;
    for(int idx = 0; idx < len_sample; idx += 2) {
        float _x = ((idx * 1.0) / len_sample) * (2.0 * M_PI);
        s[idx] = (unsigned char)((cos(_x) * r) + x);
        s[idx + 1] = (unsigned char)(sin(_x) * r + y);

        s[len - idx] = s[idx];
        s[len - idx - 1] = s[idx + 1];
    }
}

void
jitter(unsigned char *s, size_t len, float j) {
    for(int idx = 0; idx < len; idx++) {
        float r = (rand() / (RAND_MAX * 1.0)) * j - (j/2);
        s[idx] = (unsigned char)(s[idx] + r);
    }
}

void
para(unsigned char *s, size_t len, float r1, float r2, float x, float y, char flip) {
    size_t len_sample = len / 2;
    float _x = x;
    float _y = y;
    for(int idx = 0; idx < len_sample; idx += 2) {
        float x = (((idx * 1.0) / (len_sample / 2))) - 1.0;
        int idx_x = idx;
        int idx_y = idx + 1;
        if (flip) {
            idx_x = idx + 1;
            idx_y = idx;
        }
        s[idx_x] = (unsigned char)((x * r1) + _x);
        s[idx_y] = (unsigned char)((pow(x * r2, 2.0) * r1) + _y);

        s[len - idx_x] = s[idx_x];
        s[len - idx_y] = s[idx_y];
    }
}

int
main(int argc, char *argv[]) {
    int fd;	    /* sound device file descriptor */
    int arg;	/* argument for ioctl calls */
    int status; /* return status of system calls */

    /* open sound device */
    fd = open("/dev/dsp", O_RDWR);
    if (fd < 0) {
        perror("open of /dev/dsp failed");
        exit(1);
    }

    /* set sampling parameters */
    arg = SIZE;	   /* sample size */
    status = ioctl(fd, SOUND_PCM_WRITE_BITS, &arg);
    if (status == -1) perror("SOUND_PCM_WRITE_BITS ioctl failed");
    if (arg != SIZE) perror("unable to set sample size");

    arg = CHANNELS;  /* mono or stereo */
    status = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &arg);
    if (status == -1) perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
    if (arg != CHANNELS) perror("unable to set number of channels");

    arg = RATE;	   /* sampling rate */
    status = ioctl(fd, SOUND_PCM_WRITE_RATE, &arg);
    if (status == -1) perror("SOUND_PCM_WRITE_WRITE ioctl failed");

    unsigned char *s = malloc(sizeof(char) * BUFLEN);
    while (1) {
        para(s, BUFLEN, 50, 0.85, 127, 127, 1);
        write(fd, s, BUFLEN);
    }
    free(s);

    return 0;
}
