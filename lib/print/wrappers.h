#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

void _putchar(char character);
int __wrap_printf(const char* format, ...);
int __wrap_vsnprintf(char* str, size_t size, const char* format, va_list args);
int __wrap_puts(const char* str);
int __wrap_putchar(int ch);
int __wrap_putc(int ch, FILE* stream);
int __wrap_snprintf(char* str, size_t size, const char* format, ...);
int __wrap_fflush(FILE* stream);

__attribute__((__noreturn__)) void __wrap___assert(const char* file, int line, const char* e);

__attribute__((__noreturn__)) void
    __wrap___assert_func(const char* file, int line, const char* func, const char* e);

#ifdef __cplusplus
}
#endif