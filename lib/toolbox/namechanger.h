#pragma once

#define NAMECHANGER_HEADER "Flipper Name File"
#define NAMECHANGER_VERSION 1
#define NAMECHANGER_PATH EXT_PATH("dolphin/name.settings")

#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initializes the name changer. (Load name file, apply changes)
bool NameChanger_Init();

#ifdef __cplusplus
}
#endif