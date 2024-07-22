#pragma once

#include <gui/view.h>

#define MOUSE_MOVE_SHORT 5
#define MOUSE_MOVE_LONG  20

typedef struct Hid Hid;
typedef struct HidMouse HidMouse;

HidMouse* hid_mouse_alloc(Hid* bt_hid);

void hid_mouse_free(HidMouse* hid_mouse);

View* hid_mouse_get_view(HidMouse* hid_mouse);

void hid_mouse_set_connected_status(HidMouse* hid_mouse, bool connected);
