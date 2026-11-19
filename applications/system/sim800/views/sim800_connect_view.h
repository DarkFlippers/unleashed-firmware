#pragma once

#include <gui/view.h>

typedef struct Sim800ConnectView Sim800ConnectView;

Sim800ConnectView* sim800_connect_view_alloc(void);
void sim800_connect_view_free(Sim800ConnectView* instance);
View* sim800_connect_view_get_view(Sim800ConnectView* instance);

/** Rate to show, and how long the run is expected to take so the bar can be scaled. */
void sim800_connect_view_start(Sim800ConnectView* instance, uint32_t baud, uint32_t total_ms);
