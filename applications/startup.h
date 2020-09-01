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
void application_uart_write(void* p);
void application_ipc_display(void* p);
void application_ipc_widget(void* p);

const FlipperStartupApp FLIPPER_STARTUP[] = {
    #ifdef TEST
    {.app = flipper_test_app, .name = "test app"},
    #endif

    #ifdef EXAMPLE_BLINK
    {.app = application_blink, .name = "blink"},
    #endif
    #ifdef EXAMPLE_UART_WRITE
    {.app = application_uart_write, .name = "uart write"},
    #endif
    #ifdef EXAMPLE_IPC
    {.app = application_ipc_display, .name = "ipc display"},
    {.app = application_ipc_widget, .name = "ipc widget"},
    #endif
};