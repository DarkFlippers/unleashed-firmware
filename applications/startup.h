#pragma once

#include "furi.h"
#include "tests/test_index.h"

typedef struct {
    FlipperApplication app;
    const char* name;
} FlipperStartupApp;

const FlipperStartupApp FLIPPER_STARTUP[] = {
    {.app = flipper_test_app, .name = "test app"}
};