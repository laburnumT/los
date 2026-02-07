#include "stdlib.h"

#include <ctype.h>
#include <limits.h>
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

// Assumes that alphabets are contiguous, which is not guaranteed by the spec.
static int char_to_int(char c)
{
        if (c >= '0' && c <= '9')
                return c - '0';
        if (c >= 'a' && c <= 'z')
                return c - 'a';
        if (c >= 'A' && c <= 'Z')
                return c - 'A';
        return INT_MAX;
}

static unsigned long long strtoull_internal(const char *restrict nptr,
                                            char **restrict endptr, int base,
                                            int8_t *neg)
{
        for (; isspace(*nptr); ++nptr)
                ;

        if (*nptr == '+') {
                ++nptr;
        } else if (*neg == -1) {
                if (*nptr == '-')
                        ++nptr;
                else
                        *neg = 1;
        }

        uint8_t skipped_prefix = 0;
        if (base == 0) {
                if (*nptr == '0') {
                        skipped_prefix = 1;
                        ++nptr;
                        if (*nptr == 'x' || *nptr == 'X') {
                                base = 16;
                                ++nptr;
                        } else if (*nptr == 'b' || *nptr == 'B') {
                                base = 2;
                                ++nptr;
                        } else {
                                base = 8;
                        }
                } else {
                        base = 10;
                }
        }
        if (!skipped_prefix && *nptr == '0' && (base == 2 || base == 16)) {
                char sig = base == 2 ? 'b' : 'x';
                if (nptr[1] == sig || nptr[1] == sig - 'a' + 'A')
                        nptr += 2;
        }
        long long ret = 0;
        for (; char_to_int(*nptr) <= base; ++nptr)
                ret = ret * base + char_to_int(*nptr);

        if (endptr)
                *endptr = (char *restrict)nptr;

        return ret;
}

unsigned long long strtoull(const char *restrict nptr, char **restrict endptr,
                            int base)
{
        return strtoull_internal(nptr, endptr, base, &(int8_t){1});
}

long long strtoll(const char *restrict nptr, char **restrict endptr, int base)
{
        int8_t mod = -1;
        return (long long)strtoull_internal(nptr, endptr, base, &mod) * mod;
}

unsigned long int strtoul(const char *restrict nptr, char **restrict endptr,
                          int base)
{
        return (unsigned long int)strtoull(nptr, endptr, base);
}

long int strtol(const char *restrict nptr, char **restrict endptr, int base)
{
        return (long int)strtoll(nptr, endptr, base);
}
