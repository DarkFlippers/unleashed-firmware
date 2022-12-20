#pragma once

#include <gui/view.h>

#define MOUSE_MOVE_SHORT 5
#define MOUSE_MOVE_LONG 20

typedef struct HidMouseJiggler HidMouseJiggler;

HidMouseJiggler* hid_mouse_jiggler_alloc();

void hid_mouse_jiggler_free(HidMouseJiggler* hid_mouse_jiggler);

View* hid_mouse_jiggler_get_view(HidMouseJiggler* hid_mouse_jiggler);
