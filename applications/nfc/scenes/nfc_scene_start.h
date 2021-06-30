#pragma once

#include "app_scene.h"

AppScene* nfc_scene_start_alloc();

void nfc_scene_start_free(AppScene* scene);
