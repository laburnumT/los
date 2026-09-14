#include <sched.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
        puts(
            "Hello from userspace. This is the dumbest shell you'll ever see.");

        struct {
                size_t size, pos;
                char data[];
        } *buff    = malloc(sizeof(*buff) + 10 * sizeof(*buff->data));
        buff->size = 10;
        buff->pos  = 0;
        printf("> ");
        int x = 0;
        for (;;) {
                int c = getchar();
                if (c == -1)
                        continue;
                putchar(c);
                if (buff->pos == buff->size) {
                        buff->size *= 2;
                        buff = realloc(buff,
                                       sizeof(*buff) +
                                           buff->size * sizeof(*buff->data));
                }
                if (c == '\b') {
                        buff->pos--;
                        continue;
                }
                buff->data[buff->pos++] = (char)c;
                if (c == '\n') {
                        buff->data[buff->pos - 1] = '\0';
                        if (buff->data[0] == 'e' && buff->data[1] == 'c' &&
                            buff->data[2] == 'h' && buff->data[3] == 'o')
                                printf("%s\n", buff->data + 4);
                        else if (buff->data[0] == 'y' && buff->data[1] == 'i' &&
                                 buff->data[2] == 'e' && buff->data[3] == 'l' &&
                                 buff->data[4] == 'd')
                                sched_yield();
                        else if (buff->data[0] == 'x' && buff->data[1] == '=')
                                x = strtol(buff->data + 2, nullptr, 10);
                        else if (buff->data[0] == 'x')
                                printf("x is %d\n", x);
                        else if (buff->data[0] == 'e' && buff->data[1] == 'x' &&
                                 buff->data[2] == 'i' && buff->data[3] == 't')
                                break;
                        else
                                puts("Unknown command");
                        printf("> ");
                        buff->pos = 0;
                }
        }
        puts("Exiting lsh");
        return 0;
}
