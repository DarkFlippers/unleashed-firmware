#pragma once

#include "furi.h"

typedef struct {
    FlipperApplication app;
    const char* name;
    const char* libs;
} FlipperStartupApp;

#ifdef TEST
void flipper_test_app(void* p);
#endif

void application_blink(void* p);
void application_uart_write(void* p);
void application_ipc_display(void* p);
void application_ipc_widget(void* p);
void application_input_dump(void* p);

void display_u8g2(void* p);

void u8g2_example(void* p);

void input_task(void* p);

void coreglitch_demo_0(void* p);

const FlipperStartupApp FLIPPER_STARTUP[] = {
#ifndef TEST
    {.app = display_u8g2, .name = "display_u8g2", .libs = ""},
    {.app = u8g2_example, .name = "u8g2_example", .libs = "display_u8g2"},
#endif

#ifdef USE_INPUT
    {.app = input_task, .name = "input_task", .libs = ""},
#endif

// {.app = coreglitch_demo_0, .name = "coreglitch_demo_0", .libs = ""},

#ifdef TEST
    {.app = flipper_test_app, .name = "test app", .libs = ""},
#endif

#ifdef EXAMPLE_BLINK
    {.app = application_blink, .name = "blink", .libs = ""},
#endif
#ifdef EXAMPLE_UART_WRITE
    {.app = application_uart_write, .name = "uart write", .libs = ""},
#endif
#ifdef EXAMPLE_IPC
    {.app = application_ipc_display, .name = "ipc display", .libs = ""},
    {.app = application_ipc_widget, .name = "ipc widget", .libs = ""},
#endif
#ifdef EXAMPLE_INPUT_DUMP
    {.app = application_input_dump, .name = "input dump", .libs = "input_task"},
#endif
};
