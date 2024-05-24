#pragma once

#include <gui/view.h>

typedef struct Hid Hid;
typedef struct HidMouseJigglerStealth HidMouseJigglerStealth;

HidMouseJigglerStealth* hid_mouse_jiggler_stealth_alloc(Hid* bt_hid);

void hid_mouse_jiggler_stealth_free(HidMouseJigglerStealth* hid_mouse_jiggler);

View* hid_mouse_jiggler_stealth_get_view(HidMouseJigglerStealth* hid_mouse_jiggler);

void hid_mouse_jiggler_stealth_set_connected_status(
    HidMouseJigglerStealth* hid_mouse_jiggler,
    bool connected);
