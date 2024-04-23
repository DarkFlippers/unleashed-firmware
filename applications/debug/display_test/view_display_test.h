#pragma once

#include <stdint.h>
#include <gui/view.h>

typedef struct ViewDisplayTest ViewDisplayTest;

ViewDisplayTest* view_display_test_alloc(void);

void view_display_test_free(ViewDisplayTest* instance);

View* view_display_test_get_view(ViewDisplayTest* instance);
