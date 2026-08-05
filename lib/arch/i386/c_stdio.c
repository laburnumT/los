__attribute__((weak)) int c_putchar(int c)
{
        __asm__("mov eax, 0\n"
                "mov ebx, %0\n"
                "int 0x80\n"
                "mov %0, eax\n"
                : "=r"(c)
                : "r"(c)
                : "eax", "ebx");
        return c;
}

__attribute__((weak)) int c_getchar(void)
{
        int c;
        __asm__("mov eax, 1\n"
                "int 0x80\n"
                "mov %0, eax\n"
                : "=r"(c)::"eax");
        return 0;
}
