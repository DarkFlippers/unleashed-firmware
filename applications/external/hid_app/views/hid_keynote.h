#pragma once

#include <gui/view.h>

typedef struct Hid Hid;
typedef struct HidKeynote HidKeynote;

HidKeynote* hid_keynote_alloc(Hid* bt_hid);

void hid_keynote_free(HidKeynote* hid_keynote);

View* hid_keynote_get_view(HidKeynote* hid_keynote);

void hid_keynote_set_connected_status(HidKeynote* hid_keynote, bool connected);

void hid_keynote_set_orientation(HidKeynote* hid_keynote, bool vertical);