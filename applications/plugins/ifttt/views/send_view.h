#pragma once

#include <gui/view.h>

typedef struct SendView SendView;

SendView* send_view_alloc();

void send_view_free(SendView* send_view);

View* send_view_get_view(SendView* send_view);