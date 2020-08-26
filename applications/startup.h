#pragma once

#include "furi.h"

typedef struct {
    FlipperApplication app;
    const char* name;
} FlipperStartupApp;

#ifdef TEST
void flipper_test_app(void* p);
#endif

void application_blink(void* p);

const FlipperStartupApp FLIPPER_STARTUP[] = {
    #ifdef TEST
    {.app = flipper_test_app, .name = "test app"},
    #endif

    #ifdef EXAMPLE_BLINK
    {.app = application_blink, .name = "blink"},
    #endif
};