#pragma once

#include "app_scene.h"

AppScene* nfc_scene_read_card_success_alloc();

void nfc_scene_read_card_success_free(AppScene* scene);
