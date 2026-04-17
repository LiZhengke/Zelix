#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

extern void putchar(char c);

int puts(const char *__s)
{
    while (*__s)
        putchar(*__s++);
    putchar('\n');
    return 0;
}

static void print_uint(unsigned int value, unsigned int base)
{
    char buf[32];
    int i = 0;

    if (value == 0) {
        putchar('0');
        return;
    }

    while (value) {
        unsigned int digit = value % base;
        buf[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        value /= base;
    }

    while (i--) {
        putchar(buf[i]);
    }
}

static void print_uint_formatted(unsigned long value,
                                 unsigned int base,
                                 int width,
                                 int zero_pad,
                                 int uppercase)
{
    char buf[32];
    int i = 0;

    do {
        unsigned int digit = (unsigned int)(value % base);
        if (digit < 10U) {
            buf[i++] = (char)('0' + digit);
        } else {
            const char hex_base = uppercase ? 'A' : 'a';
            buf[i++] = (char)(hex_base + digit - 10U);
        }
        value /= base;
    } while ((value != 0UL) && (i < (int)sizeof(buf)));

    while (i < width) {
        putchar(zero_pad ? '0' : ' ');
        width--;
    }

    while (i--) {
        putchar(buf[i]);
    }
}

static void print_int(int value)
{
    if (value < 0) {
        putchar('-');
        print_uint((unsigned int)(-value), 10);
    } else {
        print_uint((unsigned int)value, 10);
    }
}

static void print_dec_unsigned(unsigned long val)
{
    char buf[16];
    int i = 0;

    if (val == 0) {
        putchar('0');
        return;
    }

    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }

    while (i > 0) {
        putchar(buf[--i]);
    }
}

static int vprintf_internal(const char *__restrict __format, va_list __ap);
int printf_va(const char *__restrict __format, va_list *__ap);

int printf (const char *__restrict __format, ...)
{
    va_list ap;
    int ret;

    va_start(ap, __format);
    ret = vprintf_internal(__format, ap);
    va_end(ap);
    return ret;
}

static int vprintf_internal(const char *__restrict __format, va_list __ap)
{
    for (; *__format; __format++) {
        if (*__format != '%') {
            putchar(*__format);
            continue;
        }

        __format++;  // skip %

        int zero_pad = 0;
        int width = 0;

        if (*__format == '0') {
            zero_pad = 1;
            __format++;
        }

        while ((*__format >= '0') && (*__format <= '9')) {
            width = (width * 10) + (*__format - '0');
            __format++;
        }

        // Check for 'l' length modifier
        int is_long = 0;
        if (*__format == 'l') {
            is_long = 1;
            __format++;
        }

        switch (*__format) {
        case 'c':
            putchar((char)va_arg(__ap, int));
            break;

        case 's': {
            const char *s = va_arg(__ap, const char *);
            if (!s) s = "(null)";
            while (*s) putchar(*s++);
            break;
        }

        case 'x':
        case 'X': {
            unsigned long value;

            if (is_long) {
                value = va_arg(__ap, unsigned long);
            } else {
                value = va_arg(__ap, unsigned int);
            }

            print_uint_formatted(value,
                                 16,
                                 width,
                                 zero_pad,
                                 (*__format == 'X'));
            break;
        }

        case 'p': {
            void *p = va_arg(__ap, void *);
            putchar('0');
            putchar('x');
            print_uint((unsigned int)(uintptr_t)p, 16);
            break;
        }

        case 'd': {
            if (is_long) {
                long val = va_arg(__ap, long);
                if (val < 0) {
                    putchar('-');
                    print_dec_unsigned((unsigned long)(-val));
                } else {
                    print_dec_unsigned((unsigned long)val);
                }
            } else {
                int val = va_arg(__ap, int);
                print_int(val);
            }
            break;
        }

        case 'u': {
            if (is_long) {
                unsigned long val = va_arg(__ap, unsigned long);
                print_dec_unsigned(val);
            } else {
                unsigned int val = va_arg(__ap, unsigned int);
                print_uint(val, 10);
            }
            break;
        }

        default:
            putchar('%');
            if (is_long) putchar('l');
            putchar(*__format);
            break;
        }
    }

    return 0;
}

int printf_va(const char *__restrict __format, va_list *__ap)
{
    if (__ap == NULL) {
        return -1;
    }
    return vprintf_internal(__format, *__ap);
}
