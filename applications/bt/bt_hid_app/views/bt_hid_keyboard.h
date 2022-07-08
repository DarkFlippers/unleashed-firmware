#pragma once

#include <gui/view.h>

typedef struct BtHidKeyboard BtHidKeyboard;

BtHidKeyboard* bt_hid_keyboard_alloc();

void bt_hid_keyboard_free(BtHidKeyboard* bt_hid_keyboard);

View* bt_hid_keyboard_get_view(BtHidKeyboard* bt_hid_keyboard);

void bt_hid_keyboard_set_connected_status(BtHidKeyboard* bt_hid_keyboard, bool connected);
