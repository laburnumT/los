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

// Assumes alphabet is contiguous
inline static int isupper(int c)
{
        return c >= 'A' && c <= 'Z';
}

inline static int islower(int c)
{
        return c >= 'a' && c <= 'z';
}

inline static int isalpha(int c)
{
        return isupper(c) || islower(c);
}

inline static int isalnum(int c)
{
        return isalpha(c) || isdigit(c);
}

inline static int toupper(int c)
{
        if (!islower(c))
                return c;
        return c - 'a' + 'A';
}

inline static int tolower(int c)
{
        if (!isupper(c))
                return c;
        return c - 'A' + 'a';
}

#endif
