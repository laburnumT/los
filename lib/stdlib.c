#include <stdlib.h>

#include <ctype.h>
#include <stdint.h>

int atoi(const char *nptr)
{
        int8_t sign = 0;
        if (*nptr == '-') {
                sign = -1;
                ++nptr;
        }
        int ret = 0;

        for (; isdigit(*nptr); ++nptr) {
                ret += *nptr - '0';
        }

        return ret * sign;
}
