#pragma once

#include <input/input.h>

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} GameEvent;

typedef void (*SandboxRenderCallback)(Canvas* canvas);
typedef void (*SandboxEventHandler)(GameEvent event);

void sandbox_init(
    uint8_t fps,
    SandboxRenderCallback render_callback,
    SandboxEventHandler event_handler);
void sandbox_loop();
void sandbox_loop_exit();
void sandbox_free();
