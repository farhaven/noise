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
#define RATE (1000 * 0.5) /* the sampling rate */

#define BUFLEN (1024 * 4)

unsigned char pyramid_data[] = {
    0x00, 0x5f, 0x3f, 0x00,
    0x7f, 0x7f, 0x5f, 0x5f,
    0x3f, 0x00, 0x1f, 0x7f,
    0x7f, 0x7f, 0x5f, 0x5f,
    0x00, 0x5f, 0x1f, 0x7f
};

unsigned char data_c[] = {
    0xfa, 0xc8, 0xe1, 0xe1, 0xc8, 0xfa, /* 1 -> 2 */
    0x64, 0xfa, 0x32, 0xfa, /* 2 -> 3 */
    0x19, 0xe1, 0x00, 0xc8, /* 3 -> 4 */
    0x00, 0x64, 0x00, 0x32, /* 4 -> 5 */
    0x19, 0x19, 0x32, 0x00, /* 5 -> 6 */
    0x64, 0x00, 0xc8, 0x00, /* 6 -> 7 */
    0xe1, 0x19, 0xfa, 0x32, /* 7 -> 8 */

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

void
rotate(unsigned char *s, size_t len, float angle) {
    for (int i = 0; i < len; i += 2) {
        float x = s[i];
        float y = s[i + 1];
        
        float x2 = x * cos(angle) - y * sin(angle);
        float y2 = y * cos(angle) + x * sin(angle);

        s[i] = (unsigned char) x2 + 0x70;
        s[i + 1] = (unsigned char) y2 + 0x70;
    }
}

void
circle(unsigned char *s, size_t len, float r, float x, float y) {
    for(int idx = 0; idx < len; idx += 2) {
        float _x = ((idx * 1.0) / len) * (2.0 * M_PI);
        s[idx] = (unsigned char)((cos(_x) * r) + x);
        s[idx + 1] = (unsigned char)(sin(_x) * r + y);
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
    float max_y = 255.0;
    for(int idx = 0; idx < len; idx += 2) {
        float _x = ((((idx * 1.0) / (len/ 2))) - 1.0);
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

    unsigned char *s;
    unsigned char *d;
    size_t d_size;
    float angle = 0;
    unsigned char m = 0;
    while (1) {
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
        } else if (m == 1) {
            d = data_c;
            d_size = sizeof(data_c);
        } else if (m == 2) {
            d = data_3;
            d_size = sizeof(data_3);
        } else if (m == 3) {
            d = data_p;
            d_size = sizeof(data_p);
        } else if (m == 4) {
            d = data_b;
            d_size = sizeof(data_b);
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
    }

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
