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

#define SIZE 8     /* sample size: 8 or 16 bits */
#define CHANNELS 2 /* 1 = mono 2 = stereo */
#define RATE (1000 * 2) /* the sampling rate */

#define BUFLEN (1024 * 4)

unsigned char pyramid_data[] = {
    0x00, 0x50, 0x48, 0x00,
    0x7f, 0x7f, 0x20, 0x7f,
    0x48, 0x00, 0x20, 0x7f
};

void
rotate(unsigned char *s, size_t len, float angle) {
    for (int i = 0; i < len; i += 2) {
        float x = s[i];
        float y = s[i + 1];
        
        float x2 = x * cos(angle) - y * sin(angle);
        float y2 = y * cos(angle) + x * sin(angle);

        s[i] = (unsigned char) x2 + 0x7f;
        s[i + 1] = (unsigned char) y2 + 0x7f;
    }
}

void
circle(unsigned char *s, size_t len, float r, float x, float y) {
    // size_t len_sample = len / 2;
    size_t len_sample = len;
    for(int idx = 0; idx < len_sample; idx += 2) {
        float _x = ((idx * 1.0) / len_sample) * (2.0 * M_PI);
        s[idx] = (unsigned char)((cos(_x) * r) + x);
        s[idx + 1] = (unsigned char)(sin(_x) * r + y);

        // s[len - idx] = s[idx];
        // s[len - idx - 1] = s[idx + 1];
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
para(unsigned char *s, size_t len, float r1, float r2, float r3, float x, float y, char flip_y, char exch_xy) {
    // size_t len_sample = len / 2;
    size_t len_sample = len;
    float max_y = 255.0;
    for(int idx = 0; idx < len_sample; idx += 2) {
        float _x = ((((idx * 1.0) / (len_sample / 2))) - 1.0);
        float _y = pow(_x * r3, 2.0) * r2 + y;
        _x = (_x * r1) + x;

        if (_y > max_y) {
            s[idx] = 0x00;
            s[idx + 1] = 0x00;
        } else {
            int idx_x = idx;
            int idx_y = idx + 1;
            if (exch_xy) {
                idx_x = idx + 1;
                idx_y = idx;
            }
            s[idx_x] = (unsigned char) _x;
            if (flip_y) {
                s[idx_y] = (unsigned char) ((_y - 127.0) * -1) + 127.0;
            } else {
                s[idx_y] = (unsigned char) _y;
            }
        }

        /* s[len - idx] = s[idx];
        s[len - idx - 1] = s[idx + 1];
        */
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

    for (int idx = 0; idx < sizeof(pyramid_data); idx++)
        pyramid_data[idx] *= 0.5;

    unsigned char *s = malloc(sizeof(pyramid_data));
    float angle = 0;
    float size = 0.75;
    float size_max = 0.8;
    float size_min = 0.3;
    float size_inc = 0.001;
    while (1) {
        angle += 0.001;
        size += size_inc;
        if (size > size_max) size_inc = -0.001;
        if (size < size_min) size_inc = 0.001;

        memcpy(s, pyramid_data, sizeof(pyramid_data));

        for (int idx = 0; idx < sizeof(pyramid_data); idx++)
            s[idx] *= size;

        rotate(s, sizeof(pyramid_data), angle);
        write(fd, s, sizeof(pyramid_data));
    }
    free(s);

    return 0;
}


/* Smiley face */
    /* circle(s, BUFLEN, 100, 127, 127);
    jitter(s, BUFLEN, 5);
    write(fd, s, BUFLEN);
    circle(s, BUFLEN / 2, 20, 180, 90);
    jitter(s, BUFLEN, 5);
    write(fd, s, BUFLEN / 2);
    circle(s, BUFLEN / 2, 20, 80, 90);
    jitter(s, BUFLEN, 5);
    write(fd, s, BUFLEN / 2);
    para(s, BUFLEN / 2, 50, 0.5, 5, 130, 70, 1, 0);
    jitter(s, BUFLEN, 5);
    write(fd, s, BUFLEN / 2); */
