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

void display_u8g2(void* p);

void u8g2_example(void* p);

const FlipperStartupApp FLIPPER_STARTUP[] = {
    #ifndef TEST
    {.app = display_u8g2, .name = "display_u8g2"},
    {.app = u8g2_example, .name = "u8g2_example"},
    #endif
    
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