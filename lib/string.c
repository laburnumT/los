#include "string.h"

#include <stdlib.h>

size_t strlen(const char *str)
{
        size_t len = 0;
        for (; *str++;)
                ++len;
        return len;
}

void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
        unsigned char *d       = dest;
        const unsigned char *s = src;
        for (size_t i = 0; i < n; ++i)
                d[i] = s[i];
        return dest;
}

void *memccpy(void *restrict dest, const void *restrict src, int c, size_t n)
{
        unsigned char *d       = dest;
        const unsigned char *s = src;
        for (size_t i = 0; i < n; ++i) {
                d[i] = s[i];
                if (s[i] == (unsigned char)c)
                        return d + i + 1;
        }
        return NULL;
}

char *strcpy(char *restrict dest, const char *restrict src)
{
        char *tmp = dest;
        for (; (*dest++ = *src++);)
                ;
        return tmp;
}

char *strncpy(char *restrict dest, const char *restrict src, size_t n)
{
        char *tmp = dest;
        size_t i;
        for (i = 0; i < n && src[i]; ++i)
                dest[i] = src[i];
        for (; i < n; ++i)
                dest[i] = '\0';

        return tmp;
}

char *strdup(const char *s)
{
        char *ret = malloc(strlen(s) + 1);
        if (!ret)
                return nullptr;
        strcpy(ret, s);
        return ret;
}

int strcmp(const char *s1, const char *s2)
{
        const unsigned char *c1 = (const unsigned char *)s1;
        const unsigned char *c2 = (const unsigned char *)s2;
        for (; *c1 && *c1 == *c2; ++c1, ++c2)
                ;

        return *c1 - *c2;
}

void *memset(void *s, int c, size_t n)
{
        unsigned char *s_cpy = s;
        for (size_t i = 0; i < n; ++i)
                s_cpy[i] = (unsigned char)c;
        return s;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
        const unsigned char *c1 = s1;
        const unsigned char *c2 = s2;
        for (size_t i = 0; i < n && *c1 == *c2; ++i, ++c1, ++c2)
                ;

        return c1 - c2;
}
