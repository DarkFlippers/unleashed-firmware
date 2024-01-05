#pragma once

#include <gui/view.h>

typedef struct Hid Hid;
typedef struct HidMovie HidMovie;

HidMovie* hid_movie_alloc(Hid* bt_hid);

void hid_movie_free(HidMovie* hid_movie);

View* hid_movie_get_view(HidMovie* hid_movie);

void hid_movie_set_connected_status(HidMovie* hid_movie, bool connected);
