#pragma once

#include "flipper.h"

#define FURI_LIB (const char*[])

#ifdef APP_TEST
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
void menu_task(void* p);

void coreglitch_demo_0(void* p);

void u8g2_qrcode(void* p);
void fatfs_list(void* p);
void gui_task(void* p);
void backlight_control(void* p);
void app_loader(void* p);

const FlipperStartupApp FLIPPER_STARTUP[] = {
#ifdef APP_DISPLAY
    {.app = display_u8g2, .name = "display_u8g2", .libs = {0}},
#endif

#ifdef APP_INPUT
    {.app = input_task, .name = "input_task", .libs = {0}},
#endif

#ifdef APP_GUI
    {.app = backlight_control, .name = "backlight_control", .libs = {1, FURI_LIB{"input_task"}}},
    {.app = gui_task, .name = "gui_task", .libs = {0}},
#endif

#ifdef APP_MENU
    {.app = menu_task, .name = "menu_task", .libs = {1, FURI_LIB{"gui_task"}}},
    {.app = app_loader, .name = "app_loader", .libs = {1, FURI_LIB{"menu_task"}}},
#endif

// {.app = coreglitch_demo_0, .name = "coreglitch_demo_0", .libs = ""},

#ifdef APP_TEST
    {.app = flipper_test_app, .name = "test app", .libs = {0}},
#endif

#ifdef APP_EXAMPLE_BLINK
    {.app = application_blink, .name = "blink", .libs = {0}},
#endif

#ifdef APP_EXAMPLE_UART_WRITE
    {.app = application_uart_write, .name = "uart write", .libs = {0}},
#endif

#ifdef APP_EXAMPLE_IPC
    {.app = application_ipc_display, .name = "ipc display", .libs = {0}},
    {.app = application_ipc_widget, .name = "ipc widget", .libs = {0}},
#endif

#ifdef APP_EXAMPLE_INPUT_DUMP
    {.app = application_input_dump, .name = "input dump", .libs = {1, FURI_LIB{"input_task"}}},
#endif

#ifdef APP_EXAMPLE_QRCODE
    {.app = u8g2_qrcode, .name = "u8g2_qrcode", .libs = {1, FURI_LIB{"display_u8g2"}}},
#endif

#ifdef APP_EXAMPLE_FATFS
    {.app = fatfs_list, .name = "fatfs_list", .libs = {2, FURI_LIB{"display_u8g2", "input_task"}}},
#endif

#ifdef APP_EXAMPLE_DISPLAY
    {.app = u8g2_example, .name = "u8g2_example", .libs = {1, FURI_LIB{"display_u8g2"}}},
#endif

};
