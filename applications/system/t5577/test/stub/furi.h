// Host stubs, enough to compile t5577_sniff.c away from the firmware. Test scaffolding only.
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define furi_check(x)                                                   \
    do {                                                                \
        if(!(x)) {                                                      \
            printf("furi_check failed: %s:%d %s\n", __FILE__, __LINE__, #x); \
            exit(2);                                                    \
        }                                                               \
    } while(0)

#define FURI_CRITICAL_ENTER() ((void)0)
#define FURI_CRITICAL_EXIT()  ((void)0)
