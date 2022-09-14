#pragma once

#include <gui/view.h>

typedef struct BtHidMouse BtHidMouse;

BtHidMouse* bt_hid_mouse_alloc();

void bt_hid_mouse_free(BtHidMouse* bt_hid_mouse);

View* bt_hid_mouse_get_view(BtHidMouse* bt_hid_mouse);

void bt_hid_mouse_set_connected_status(BtHidMouse* bt_hid_mouse, bool connected);
