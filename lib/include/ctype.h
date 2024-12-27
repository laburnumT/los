#ifndef LOS_LIB_CTYPE_H
#define LOS_LIB_CTYPE_H

inline static int isdigit(int c)
{
        return c >= '0' && c <= '9';
}

inline static int isspace(int c)
{
        return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' ||
               c == '\v';
}

#endif
