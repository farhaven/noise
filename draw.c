#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#include "draw.h"

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

