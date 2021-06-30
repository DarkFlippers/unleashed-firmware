#pragma once

#include "app_scene.h"

AppScene* nfc_scene_debug_read_mifare_ul_alloc();

void nfc_scene_debug_read_mifare_ul_free(AppScene* scene);
