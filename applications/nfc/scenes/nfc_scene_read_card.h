#pragma once

#include "app_scene.h"

AppScene* nfc_scene_read_card_alloc();

void nfc_scene_read_card_free(AppScene* scene);
