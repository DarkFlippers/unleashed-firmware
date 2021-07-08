#pragma once

#include "app_scene.h"

AppScene* nfc_scene_read_mifare_ul_success_alloc();

void nfc_scene_read_mifare_ul_success_free(AppScene* scene);
