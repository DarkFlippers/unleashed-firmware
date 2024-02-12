#pragma once

#include <gui/view.h>

typedef struct JsConsoleView JsConsoleView;

JsConsoleView* console_view_alloc(void);

void console_view_free(JsConsoleView* console_view);

View* console_view_get_view(JsConsoleView* console_view);

void console_view_print(JsConsoleView* console_view, const char* text);
