#ifndef LOS_LIB_STRING_H
#define LOS_LIB_STRING_H

#include <stddef.h>

size_t strlen(const char *str);
void *memcpy(void *restrict dest, const void *restrict src, size_t n);
void *memccpy(void *restrict dest, const void *restrict src, int c, size_t n);
// void *memmove(void *dest, const void *src, size_t n);
char *strcpy(char *restrict dest, const char *restrict src);
char *strncpy(char *restrict dest, const char *restrict src, size_t n);
char *strdup(const char *s);
int strcmp(const char *s1, const char *s2);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

#endif
