#ifndef LOS_LIB_STDLIB_H
#define LOS_LIB_STDLIB_H

#include <stddef.h>

int atoi(const char *nptr);
long int strtol(const char *restrict nptr, char **restrict endptr, int base);
unsigned long int strtoul(const char *restrict nptr, char **restrict endptr,
                          int base);
long long strtoll(const char *restrict nptr, char **restrict endptr, int base);
unsigned long long strtoull(const char *restrict nptr, char **restrict endptr,
                            int base);

void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

#endif
