#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t id;
    uint32_t state;
    const void (*on_enter)(void* context);
    const bool (*on_event)(void* context, uint32_t event);
    const void (*on_exit)(void* context);
} AppScene;
