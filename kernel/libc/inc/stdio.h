#pragma once

#include <stdarg.h>

int putchar(int c);
int puts(const char *s);
int printf(const char *format, ...);
int printf_va(const char *format, va_list *ap);
