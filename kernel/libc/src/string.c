#include <stddef.h>

#include <string.h>

void *memset(void *s, int c, size_t n)
{
    unsigned char *p = s;
    while (n--)
        *p++ = (unsigned char)c;
    return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--)
        *d++ = *s++;
    return dest;
}

int memcmp(const void *s1,
           const void *s2,
           size_t n)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;

    while (n--) {
        if (*p1 != *p2) {
            return (int)*p1 - (int)*p2;
        }

        p1++;
        p2++;
    }

    return 0;
}

char *strchr(const char *s,
             int c)
{
    while (*s) {
        if (*s == (char)c) {
            return (char *)s;
        }

        s++;
    }

    return (c == '\0') ? (char *)s : NULL;
}
