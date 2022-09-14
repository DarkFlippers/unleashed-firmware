#include "wrappers.h"

#include <stdbool.h>
#include <stdarg.h>
#include <furi/core/check.h>
#include <furi/core/thread.h>
#include <furi/core/common_defines.h>
#include "printf_tiny.h"

void _putchar(char character) {
    furi_thread_stdout_write(&character, 1);
}

int __wrap_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vprintf_(format, args);
    va_end(args);

    return ret;
}

int __wrap_vsnprintf(char* str, size_t size, const char* format, va_list args) {
    return vsnprintf_(str, size, format, args);
}

int __wrap_puts(const char* str) {
    size_t size = furi_thread_stdout_write(str, strlen(str));
    size += furi_thread_stdout_write("\n", 1);
    return size;
}

int __wrap_putchar(int ch) {
    size_t size = furi_thread_stdout_write((char*)&ch, 1);
    return size;
}

int __wrap_putc(int ch, FILE* stream) {
    UNUSED(stream);
    size_t size = furi_thread_stdout_write((char*)&ch, 1);
    return size;
}

int __wrap_snprintf(char* str, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = __wrap_vsnprintf(str, size, format, args);
    va_end(args);

    return ret;
}

int __wrap_fflush(FILE* stream) {
    UNUSED(stream);
    furi_thread_stdout_flush();
    return 0;
}

__attribute__((__noreturn__)) void __wrap___assert(const char* file, int line, const char* e) {
    UNUSED(file);
    UNUSED(line);
    // TODO: message file and line number
    furi_crash(e);
}

__attribute__((__noreturn__)) void
    __wrap___assert_func(const char* file, int line, const char* func, const char* e) {
    UNUSED(file);
    UNUSED(line);
    UNUSED(func);
    // TODO: message file and line number
    furi_crash(e);
}