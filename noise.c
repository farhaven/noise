#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/soundcard.h>

#include <math.h>

#include "misc.h"
#include "draw.h"
#include "data.h"

int
main(int argc, char *argv[]) {
    int fd, arg, status;
    unsigned char *s, *d;
    size_t d_size;
    float angle = 0;
    unsigned char m = 0;

    fd = open("/dev/dsp", O_RDWR);
    if (fd < 0) {
        perror("open of /dev/dsp failed");
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

    for (int idx = 0; idx < sizeof(pyramid_data); idx++)
        pyramid_data[idx] *= 0.5;

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
