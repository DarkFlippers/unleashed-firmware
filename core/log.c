#define _GNU_SOURCE

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "log.h"
#include "furi.h"

#define PRINT_STR_SIZE 64

void fuprintf(FuriRecordSubscriber* f, const char* format, ...) {
    char buffer[PRINT_STR_SIZE];

    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    furi_write(f, buffer, strlen(buffer));
}

FuriRecordSubscriber* get_default_log() {
    return furi_open("tty", false, false, NULL, NULL, NULL);
}