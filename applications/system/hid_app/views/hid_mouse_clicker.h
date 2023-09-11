#pragma once

#include <gui/view.h>

typedef struct Hid Hid;
typedef struct HidMouseClicker HidMouseClicker;

HidMouseClicker* hid_mouse_clicker_alloc(Hid* bt_hid);

void hid_mouse_clicker_free(HidMouseClicker* hid_mouse_clicker);

View* hid_mouse_clicker_get_view(HidMouseClicker* hid_mouse_clicker);

void hid_mouse_clicker_set_connected_status(HidMouseClicker* hid_mouse_clicker, bool connected);
