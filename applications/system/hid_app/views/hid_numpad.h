#pragma once

#include <gui/view.h>

typedef struct Hid Hid;
typedef struct HidNumpad HidNumpad;

HidNumpad* hid_numpad_alloc(Hid* bt_hid);

void hid_numpad_free(HidNumpad* hid_numpad);

View* hid_numpad_get_view(HidNumpad* hid_numpad);

void hid_numpad_set_connected_status(HidNumpad* hid_numpad, bool connected);
