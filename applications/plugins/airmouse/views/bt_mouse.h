#pragma once

#include <gui/view.h>
#include <gui/view_dispatcher.h>

typedef struct BtMouse BtMouse;

BtMouse* bt_mouse_alloc(ViewDispatcher* view_dispatcher);

void bt_mouse_free(BtMouse* bt_mouse);

View* bt_mouse_get_view(BtMouse* bt_mouse);

void bt_mouse_set_connected_status(BtMouse* bt_mouse, bool connected);
