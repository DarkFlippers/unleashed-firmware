#pragma once

#include <gui/view.h>

typedef struct HidMusicMacos HidMusicMacos;

HidMusicMacos* hid_music_macos_alloc();

void hid_music_macos_free(HidMusicMacos* hid_music_macos);

View* hid_music_macos_get_view(HidMusicMacos* hid_music_macos);

void hid_music_macos_set_connected_status(HidMusicMacos* hid_music_macos, bool connected);
