#include <stdio.h>

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __is_libk
        #include <kernel/keyboard.h>
        #include <kernel/tty.h>
#endif

enum : uint8_t {
        FMT_LEFT_JUSTIFIED = 0x1 << 0,
        FMT_LEADING_SIGN   = 0x1 << 1,
        FMT_SPACE          = 0x1 << 2,
        FMT_ALTERNATE_FORM = 0x1 << 3,
        FMT_PAD_0          = 0x1 << 4,
};

struct stdio_format_type {
        uint8_t flag;
        uint8_t field_width;
        uint8_t precision;
        uint8_t length;
        enum stdio_conv_format : uint8_t {
                CONV_SDEC,
                CONV_UBIN,
                CONV_UOCT,
                CONV_UDEC,
                CONV_UHEX_LOW,
                CONV_UHEX_UP,
                CONV_POINT,
                CONV_CHAR,
                CONV_STR,
                CONV_PERCENT,
                // TODO(laburnumT): Support floats
        } conversion;
};

static void parse_format(const char *restrict *restrict format,
                         struct stdio_format_type *fmt_type)
{
        *fmt_type       = (struct stdio_format_type){};
        const char *fmt = *format;
        // Detect flag
        for (;; ++fmt) {
                if (*fmt == '-')
                        fmt_type->flag |= FMT_LEFT_JUSTIFIED;
                else if (*fmt == '+')
                        fmt_type->flag |= FMT_LEADING_SIGN;
                else if (*fmt == ' ')
                        fmt_type->flag |= FMT_SPACE;
                else if (*fmt == '#')
                        fmt_type->flag |= FMT_ALTERNATE_FORM;
                else if (*fmt == '0')
                        fmt_type->flag |= FMT_PAD_0;
                else
                        break;
        }

        // Detect field width
        fmt_type->field_width = 1;
        if (*fmt == '*') {
                fmt_type->field_width = 0;
                ++fmt;
        } else if (isdigit(*fmt)) {
                char buff[4] = {};
                size_t i     = 0;
                for (i = 0; isdigit(*fmt); ++i, ++fmt)
                        buff[i] = *fmt;
                buff[i]               = '\0';
                fmt_type->field_width = atoi(buff);
        }

        // Detect precision
        fmt_type->precision = 0;
        if (*fmt == '.') {
                ++fmt;
                if (*fmt == '*') {
                        // fmt_type->precision = 0;
                        ++fmt;
                } else {
                        char buff[3];
                        for (size_t i = 0; isdigit(*fmt); ++i, ++fmt)
                                buff[i] = *fmt;
                        fmt_type->precision = atoi(buff);
                }
        }

        // Detect length modifier
        fmt_type->length = 0;
        switch (*fmt++) {
        case 'h':
                if (*fmt == 'h') {
                        fmt_type->length = sizeof(char);
                        ++fmt;
                } else {
                        fmt_type->length = sizeof(short int);
                }
                break;
        case 'l':
                if (*fmt == 'l') {
                        fmt_type->length = sizeof(long long int);
                        ++fmt;
                } else {
                        fmt_type->length = sizeof(long int);
                }
                break;
        case 'j':
                fmt_type->length = sizeof(intmax_t);
                break;
        case 'z':
                fmt_type->length = sizeof(size_t);
                break;
        case 'w': {
                // TODO(laburnumT): Support fast minumum width
                if (*fmt == 'f')
                        ++fmt;

                char buff[3];
                for (size_t i = 0; isdigit(*fmt); ++i, ++fmt)
                        buff[i] = *fmt;
                switch (atoi(buff)) {
                case 8:
                        fmt_type->length = sizeof(int8_t);
                        break;
                case 16:
                        fmt_type->length = sizeof(int16_t);
                        break;
                case 32:
                        fmt_type->length = sizeof(int32_t);
                        break;
                case 64:
                        fmt_type->length = sizeof(int64_t);
                        break;
                default:
                        unreachable();
                }
                break;
        }
        case 'L':
                fmt_type->length = sizeof(long double);
                break;
        default:
                --fmt;
                break;
        }

        switch (*fmt++) {
        case 'd':
        case 'i':
                fmt_type->conversion = CONV_SDEC;
                break;
        case 'b':
        case 'B':
                fmt_type->conversion = CONV_UBIN;
                break;
        case 'o':
                fmt_type->conversion = CONV_UOCT;
                break;
        case 'x':
                fmt_type->conversion = CONV_UHEX_LOW;
                break;
        case 'X':
                fmt_type->conversion = CONV_UHEX_UP;
                break;
        case 'c':
                fmt_type->conversion = CONV_CHAR;
                break;
        case 's':
                fmt_type->conversion = CONV_STR;
                break;
        case 'p':
                fmt_type->conversion = CONV_POINT;
                break;
        case '%':
                fmt_type->conversion = CONV_PERCENT;
                break;
        default:
                break;
        }

        if (fmt_type->length == 0) {
                if (fmt_type->conversion < CONV_POINT)
                        fmt_type->length = sizeof(int);
                else if (fmt_type->conversion == CONV_POINT)
                        fmt_type->length = sizeof(uintptr_t);
        }

        *format = fmt;
}

static void rev_string(char *restrict buff, size_t len)
{
        if (len == 0)
                len = strlen(buff);
        for (size_t i = 0; i < len / 2; ++i) {
                char tmp          = buff[i];
                buff[i]           = buff[len - i - 1];
                buff[len - i - 1] = tmp;
        }
}

// TODO(laburnumT) Move to commented out version when GCC supports c23 typeof

// #define is_signed_type(t) \
//         _Generic(typeof(t), \
//             int8_t: 1, \
//             int16_t: 1, \
//             int32_t: 1, \
//             int64_t: 1, \ default: 0)

// #define get_larger_type(t1, t2) \
//         typeof(sizeof(t1) > sizeof(t2) ? (typeof(t1)){} : (typeof(t2)){})

// #define int_or_larger(t)                                                       \
//         typeof(is_signed_type(t) ? (get_larger_type(t, int)){}                 \
//                                  : (get_larger_type(t, unsigned int)){})

#define stdio_get_larger_type(t1, t2)                                          \
        typeof(sizeof(t1) > sizeof(t2) ? (typeof(t1)){} /* NOLINT */           \
                                       : (typeof(t2)){})

#define intN_to_string(t)                                                      \
        static int t##_to_str(                                                 \
            char *restrict buff, const struct stdio_format_type *fmt, t val)   \
        {                                                                      \
                static const char set_table[] = {                              \
                    '0',                                                       \
                    '1',                                                       \
                    '2',                                                       \
                    '3',                                                       \
                    '4',                                                       \
                    '5',                                                       \
                    '6',                                                       \
                    '7',                                                       \
                    '8',                                                       \
                    '9',                                                       \
                    'a',                                                       \
                    'b',                                                       \
                    'c',                                                       \
                    'd',                                                       \
                    'e',                                                       \
                    'f',                                                       \
                };                                                             \
                uint8_t mod;                                                   \
                switch (fmt->conversion) {                                     \
                case CONV_SDEC:                                                \
                case CONV_UDEC:                                                \
                        mod = 10;                                              \
                        break;                                                 \
                case CONV_UHEX_LOW:                                            \
                case CONV_UHEX_UP:                                             \
                case CONV_POINT:                                               \
                        mod = 16;                                              \
                        break;                                                 \
                case CONV_UBIN:                                                \
                        mod = 2;                                               \
                        break;                                                 \
                default:                                                       \
                        unreachable();                               \
                }                                                              \
                size_t cnt = 0;                                                \
                                                                               \
                if (!val) {                                                    \
                        buff[0] = '0';                                         \
                        buff[1] = '\0';                                        \
                        return 1;                                              \
                }                                                              \
                uint8_t is_neg = fmt->conversion == CONV_SDEC && val < 0;      \
                if (is_neg)                                                    \
                        val = -val;                                            \
                for (cnt = 0; val > 0; val /= mod) {                           \
                        buff[cnt++] = set_table[val % mod];                    \
                        if (fmt->conversion == CONV_UHEX_UP)                   \
                                buff[cnt - 1] = toupper(buff[cnt - 1]);        \
                }                                                              \
                if (is_neg)                                                    \
                        buff[cnt++] = '-';                                     \
                buff[cnt] = '\0';                                              \
                rev_string(buff, cnt);                                         \
                                                                               \
                return (int)cnt;                                               \
        }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
intN_to_string(uint8_t);
intN_to_string(int8_t);
intN_to_string(uint16_t);
intN_to_string(int16_t);
intN_to_string(uint32_t);
intN_to_string(int32_t);
intN_to_string(uint64_t);
intN_to_string(int64_t);
#pragma GCC diagnostic pop

static int num_fmt_to_str(char *restrict buff,
                          const struct stdio_format_type *fmt, va_list *va)
{
        if (fmt->conversion == CONV_SDEC) {
                switch (fmt->length) {
                case sizeof(int64_t):
                        return int64_t_to_str(
                            buff,
                            fmt,
                            va_arg(*va, stdio_get_larger_type(int64_t, int)));
                case sizeof(int32_t):
                        return int32_t_to_str(
                            buff,
                            fmt,
                            va_arg(*va, stdio_get_larger_type(int32_t, int)));
                case sizeof(int16_t):
                        return int16_t_to_str(
                            buff,
                            fmt,
                            va_arg(*va, stdio_get_larger_type(int16_t, int)));
                case sizeof(int8_t):
                default:
                        return int8_t_to_str(buff, fmt, va_arg(*va, int));
                }
        }

        switch (fmt->length) {
        case sizeof(uint64_t):
                return uint64_t_to_str(
                    buff,
                    fmt,
                    va_arg(*va, stdio_get_larger_type(uint64_t, unsigned int)));
        case sizeof(uint32_t):
                return uint32_t_to_str(
                    buff,
                    fmt,
                    va_arg(*va, stdio_get_larger_type(uint32_t, unsigned int)));
        case sizeof(uint16_t):
                return uint16_t_to_str(
                    buff,
                    fmt,
                    va_arg(*va, stdio_get_larger_type(uint16_t, unsigned int)));
        case sizeof(uint8_t):
        default:
                return uint8_t_to_str(buff, fmt, va_arg(*va, unsigned int));
        }
}

static int fmt_to_str(char *restrict buff, const struct stdio_format_type *fmt,
                      va_list *va)
{
        size_t cnt = 0;

        if (fmt->conversion < CONV_CHAR)
                return num_fmt_to_str(buff, fmt, va);

        switch (fmt->conversion) {
        case CONV_CHAR:
                buff[0] = (char)va_arg(*va, int);
                buff[1] = '\0';
                return 1;
        case CONV_STR:
                strcpy(buff, va_arg(*va, char *));
                return (int)strlen(buff);
        default:
                unreachable();
        }

        return (int)cnt;
}

int putchar(int c)
{
#ifdef __is_libk
        tty_putchar((char)c);
#else
        #error NOT IMPLEMENTED
#endif
        return c;
}

int getchar(void)
{
#ifdef __is_libk
        return keyboard_getchar();
#else
        #error NOT IMPLEMENTED
#endif
}

static int put_str(const char *s)
{
        for (; *s; ++s) {
                if (putchar(*s) != *s)
                        return EOF;
        }
        return 0;
}

int puts(const char *s)
{
        put_str(s);
        putchar('\n');

        return 0;
}

int vprintf(const char *restrict format, va_list arg)
{
        size_t cnt = 0;
        for (cnt = 0; *format; ++format) {
                if (*format != '%') {
                        ++cnt;
                        putchar(*format);
                        continue;
                }
                ++format;
                struct stdio_format_type fmt_type = {};
                parse_format(&format, &fmt_type);
                --format;
                if (fmt_type.conversion == CONV_PERCENT) {
                        ++cnt;
                        putchar('%');
                        continue;
                }
                char buff[100];
                fmt_to_str(buff, &fmt_type, &arg);
                put_str(buff);
        }

        return (int)cnt;
}

int printf(const char *restrict format, ...)
{
        va_list arg;
        va_start(arg, format);

        int cnt = vprintf(format, arg);

        va_end(arg);

        return cnt;
}
