#pragma once

#include <gui/view.h>

typedef struct Hid Hid;
typedef struct HidKeynoteVertical HidKeynoteVertical;

HidKeynoteVertical* hid_keynote_vertical_alloc(Hid* bt_hid);

void hid_keynote_vertical_free(HidKeynoteVertical* hid_keynote_vertical);

View* hid_keynote_vertical_get_view(HidKeynoteVertical* hid_keynote_vertical);

void hid_keynote_vertical_set_connected_status(
    HidKeynoteVertical* hid_keynote_vertical,
    bool connected);
