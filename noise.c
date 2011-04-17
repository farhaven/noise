#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/soundcard.h>

#include <math.h>

#include "draw.h"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#define SIZE 8     /* sample size: 8 or 16 bits */
#define CHANNELS 2 /* 1 = mono 2 = stereo */
#define RATE (1000 * 0.5) /* the sampling rate */

#define BUFLEN (1024 * 4)

/* {{{ precomputed data for letters */
unsigned char pyramid_data[] = {
    0x00, 0x5f, 0x3f, 0x00,
    0x7f, 0x7f, 0x5f, 0x5f,
    0x3f, 0x00, 0x1f, 0x7f,
    0x7f, 0x7f, 0x5f, 0x5f,
    0x00, 0x5f, 0x1f, 0x7f
};

unsigned char data_c[] = {
    0xfa, 0xc8, 0xe1, 0xe1, 0xc8, 0xfa,
    0x64, 0xfa, 0x32, 0xfa,
    0x19, 0xe1, 0x00, 0xc8,
    0x00, 0x64, 0x00, 0x32,
    0x19, 0x19, 0x32, 0x00,
    0x64, 0x00, 0xc8, 0x00,
    0xe1, 0x19, 0xfa, 0x32,

    0xe1, 0x19, 0xc8, 0x00,
    0x64, 0x00, 0x32, 0x00,
    0x19, 0x19, 0x00, 0x32,
    0x00, 0x64, 0x00, 0xc8,
    0x19, 0xe1, 0x32, 0xfa,
    0x64, 0xfa, 0xc8, 0xfa,
    0xe1, 0xe1
};

unsigned char data_3[] = {
    0x00, 0xbf, 0x40, 0xff,
    0x7f, 0xff, 0xbf, 0xbf,
    0x7f, 0x7f, 0xbf, 0x40,
    0x7f, 0x00, 0x40, 0x00,
    0x00, 0x40,

    0x40, 0x00, 0x7f, 0x00,
    0xbf, 0x40, 0x7f, 0x7f,
    0xbf, 0xbf, 0x7f, 0xff,
    0x40, 0xff
};

unsigned char data_p[] = {
    0x00, 0x00, 0x00, 0x7f,
    0x40, 0x7f, 0x7f, 0xbf,
    0x40, 0xff, 0x00, 0xff
};

unsigned char data_b[] = {
    0x00, 0x00, 0x40, 0x00,
    0x7f, 0x40, 0x40, 0x7f,
    0x00, 0x7f, 0x00, 0xff,
    0x40, 0xff, 0x7f, 0xbf,
    0x40, 0x7f, 0x00, 0x7f,
    0x00, 0x00,

    0x00, 0x7f, 0x40, 0x7f,
    0x7f, 0xbf, 0x40, 0xff,
    0x00, 0xff, 0x00, 0x7f,
    0x40, 0x7f, 0x7f, 0x40,
    0x40, 0x00
};
/* }}} */

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

    for (int idx = 0; idx < sizeof(pyramid_data); idx++)
        pyramid_data[idx] *= 0.5;

    unsigned char *s;
    unsigned char *d;
    size_t d_size;
    float angle = 0;
    unsigned char m = 0;
    while (1) {
        switch(m) {
            case 1:
                d = data_c;
                d_size = sizeof(data_c);
                break;
            case 2:
                d = data_3;
                d_size = sizeof(data_3);
                break;
            case 3:
                d = data_p;
                d_size = sizeof(data_p);
                break;
            case 4:
                d = data_b;
                d_size = sizeof(data_b);
                break;
        }
        if ((m > 0) && (m <= 4)) {
            s = malloc(d_size);
            memcpy(s, d, d_size);
            angle += 0.05;
            float size = (sin(angle) + 1) * 0.125;
            if (size <= 0.01) {
                m++;
                angle = 0;
                continue;
            }

            for (int idx = 0; idx < d_size; idx++) {
                s[idx] = 255 - s[idx];
                s[idx] *= size;
            }
            write(fd, s, d_size);
            if (m == 3)
                write(fd, s, d_size);
            free(s);
        } else if (m == 6) { 
            s = malloc(sizeof(pyramid_data));
            float size = (log((angle + 0.01) * 10) - 1) * 0.1;
            angle += 1;
            if (size >= 0.75) {
                angle = 0;
                m++;
                continue;
            }

            memcpy(s, pyramid_data, sizeof(pyramid_data));
            for (int idx = 0; idx < sizeof(pyramid_data); idx++)
                s[idx] *= size;
            write(fd, s, sizeof(pyramid_data));

            free(s);
        } else if ((m > 6) && (m <= 8)) {
            s = malloc(sizeof(pyramid_data));
            angle += 0.01;
            if (angle > (2.0 * M_PI)) {
                angle = 0;
                m++;
            }
            memcpy(s, pyramid_data, sizeof(pyramid_data));

            for (int idx = 0; idx < sizeof(pyramid_data); idx++)
                s[idx] *= 0.75;

            rotate(s, sizeof(pyramid_data), angle);
            write(fd, s, sizeof(pyramid_data));
            free(s);
        } else if (m > 8) {
            exit(0);
        }
        if ((m == 0) || (m == 5)) {
            s = malloc(sizeof(pyramid_data));
            angle += 0.01;
            float size = (sin(angle) + 1) * 0.5;
            if (size <= 0.01) {
                m++;
                angle = 0;
                continue;
            }

            memcpy(s, pyramid_data, sizeof(pyramid_data));

            for (int idx = 0; idx < sizeof(pyramid_data); idx++)
                s[idx] *= size;

            write(fd, s, sizeof(pyramid_data));
            free(s);
        }
    }

    return 0;
}
