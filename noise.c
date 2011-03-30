#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/soundcard.h>

#define MAX(x, y) ((x > y) ? x : y)
#define ABS(x) ((x >= 0) ? x : -x)

#define SIZE 8     /* sample size: 8 or 16 bits */
#define CHANNELS 2 /* 1 = mono 2 = stereo */
#define RATE (1000 * 10) /* the sampling rate */
#define BUFLEN 1024

int
main(int argc, char *argv[]) {
    int fd;	    /* sound device file descriptor */
    int arg;	/* argument for ioctl calls */
    int status; /* return status of system calls */
    char *buf = malloc(0);
    char *sidebuf;
    unsigned int len_b = 0;
    unsigned int len_s = 0;
    int x, y, max_x = 0, max_y = 0;
    int dir_x, dir_y;

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

    while (!feof(stdin)) {
        char *line = (char *) calloc(sizeof(char), BUFLEN);
        fgets(line, BUFLEN, stdin);
        if (line[0] == '\0') {
            free(line);
            break;
        }
        if ((line[0] == '\n') || (line[0] == '#')) {
            free(line);
            continue;
        }
        char *p = strchr(line, '\n');
        if (p) *p = 0x00;

        unsigned char o = strtok(line, " ")[0];
        if (o == 's') {
            free(line);
            sidebuf = malloc(0);
            len_s = 0;
            continue;
        }

        unsigned char x = atoi(strtok(NULL, " "));
        unsigned char y = atoi(strtok(NULL, " "));
        max_x = MAX(max_x, x);
        max_y = MAX(max_y, y);

        if (o == 'e') {
            for (int c = 0; c < len_s; c += 2) {
                sidebuf[c] += x;
                sidebuf[c+1] += y;
            }
            for (int c = 0; c < 1; c++) {
                buf = realloc(buf, len_b + len_s);
                memcpy(buf + len_b, sidebuf, len_s);
                len_b += len_s;
            }
            free(sidebuf);
            free(line);
            continue;
        }
        free(line);

        fprintf(stdout, "o=%c x=%02d y=%02d\n", o, x, y);

        for (int l = 0; l < ((o == 'm') ? 1 : 2); l++) {
            sidebuf = realloc(sidebuf, len_s + 2);
            sidebuf[len_s] = x;
            sidebuf[len_s + 1] = y;
            len_s += 2;
        }
    }

    char *field = malloc(sizeof(char) * len_b);
    int field_len = len_b;
    memcpy(field, buf, len_b);
    free(buf);

    fprintf(stderr, "%d\n", max_y);

    x = max_x / 3;
    y = max_y / 2;
    dir_x = 1;
    dir_y = 1;

    while (1) {
        if (x > max_x - 0x0F) dir_x = -1;
        if (x == 0x00) dir_x = 1;

        if (y > max_y - 0x0F ) dir_y = -1;
        if (y == 0x00) dir_y = 1;
        x += dir_x;
        y += dir_y;

        char s[] = { 0x00 + x, 0x00 + y,
                     0x0F + x, 0x00 + y,
                     0x0F + x, 0x0F + y,
                     0x00 + x, 0x0F + y,
        };

        for (int c = 0; c < 2; c++)
            status = write(fd, field, field_len);
        for (int c = 0; c < 4; c++)
            write(fd, s, sizeof(s));
        for (int c = 0; c < 2; c++)
            status = write(fd, field, field_len);
    }

    free(buf);

    return 0;
}
