#pragma once

#include <gui/view.h>

typedef struct Hid Hid;
typedef struct HidKeyboard HidKeyboard;

HidKeyboard* hid_keyboard_alloc(Hid* bt_hid);

void hid_keyboard_free(HidKeyboard* hid_keyboard);

View* hid_keyboard_get_view(HidKeyboard* hid_keyboard);

void hid_keyboard_set_connected_status(HidKeyboard* hid_keyboard, bool connected);
