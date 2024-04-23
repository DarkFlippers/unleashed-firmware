#pragma once

#include <gui/view.h>

typedef struct Hid Hid;
typedef struct HidMedia HidMedia;

HidMedia* hid_media_alloc(Hid* hid);

void hid_media_free(HidMedia* hid_media);

View* hid_media_get_view(HidMedia* hid_media);

void hid_media_set_connected_status(HidMedia* hid_media, bool connected);
