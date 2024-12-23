#include <stdio.h>

#include <stdarg.h>
#include <stdint.h>

#ifdef __is_libk
        #include <kernel/tty.h>
#endif

static int uint_to_str(char *buffer, uint32_t num)
{
        if (!num) {
                buffer[0] = '0';
                buffer[1] = '\0';
                return 1;
        }
        size_t cnt = 0;
        for (cnt = 0; num > 0; num /= 10)
                buffer[cnt++] = (char)(num % 10 + '0');

        for (size_t i = 0; i < cnt / 2; ++i) {
                char tmp            = buffer[i];
                buffer[i]           = buffer[cnt - i - 1];
                buffer[cnt - i - 1] = tmp;
        }
        buffer[cnt++] = '\0';

        return (int)cnt;
}

static int int_to_str(char *buffer, int32_t num)
{
        if (!num) {
                buffer[0] = '0';
                buffer[1] = '\0';
                return 1;
        }
        uint8_t is_neg = num < 0;
        size_t cnt     = 0;
        if (is_neg)
                num = -num;
        for (cnt = 0; num > 0; num /= 10)
                buffer[cnt++] = (char)(num % 10 + '0');
        if (is_neg)
                buffer[cnt++] = '-';

        for (size_t i = 0; i < cnt / 2; ++i) {
                char tmp            = buffer[i];
                buffer[i]           = buffer[cnt - i - 1];
                buffer[cnt - i - 1] = tmp;
        }
        buffer[cnt++] = '\0';

        return (int)cnt;
}

static int uintptr_t_to_str(char *buffer, uintptr_t num)
{
        if (!num) {
                buffer[0] = '0';
                buffer[1] = '\0';
                return 1;
        }
        size_t cnt = 0;
        for (cnt = 0; num > 0; num /= 10)
                buffer[cnt++] = (char)(num % 10 + '0');

        for (size_t i = 0; i < cnt / 2; ++i) {
                char tmp            = buffer[i];
                buffer[i]           = buffer[cnt - i - 1];
                buffer[cnt - i - 1] = tmp;
        }
        buffer[cnt++] = '\0';

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

int printf(const char *restrict format, ...)
{
        va_list args;
        va_start(args, format);
        size_t cnt = 0;

        for (cnt = 0; *format; ++format) {
                if (*format != '%') {
                        putchar(*format);
                        continue;
                }
                ++format;
                switch (*format) {
                case '%':
                        putchar(*format);
                        break;
                case 'd': {
                        char buff[15];
                        int_to_str(buff, va_arg(args, int));
                        put_str(buff);
                        break;
                case 'u': {
                        char buff[15];
                        uint_to_str(buff, va_arg(args, unsigned int));
                        put_str(buff);
                        break;
                }
                case 'p': {
                        char buff[15];
                        uintptr_t_to_str(buff, (uintptr_t)va_arg(args, void *));
                        put_str(buff);
                        break;
                }
                case 'c':
                        putchar(va_arg(args, int));
                        break;
                case 's':
                        put_str(va_arg(args, char *));
                        break;
                default:
                        putchar('%');
                        putchar(*format);
                        break;
                }
                }
        }

        va_end(args);

        return (int)cnt;
}
