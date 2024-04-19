#pragma once

#include <gui/view.h>

typedef struct InfraredDebugView InfraredDebugView;

InfraredDebugView* infrared_debug_view_alloc(void);
void infrared_debug_view_free(InfraredDebugView* debug_view);

View* infrared_debug_view_get_view(InfraredDebugView* debug_view);
void infrared_debug_view_set_text(InfraredDebugView* debug_view, const char* fmt, ...);
