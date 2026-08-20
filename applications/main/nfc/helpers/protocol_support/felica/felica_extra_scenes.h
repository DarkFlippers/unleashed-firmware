/**
 * @file felica_extra_scenes.h
 * @brief Indices of the Felica scenes that live in the plugin.
 *
 * Shared between the thunks in the app (scenes/nfc_scene_felica_*.c), which name the index, and
 * the plugin, which supplies the handlers.
 */
#pragma once

#include "../nfc_protocol_support_base.h"

typedef enum {
    FelicaExtraSceneMoreInfo,
    FelicaExtraSceneSystem,

    FelicaExtraSceneNum,
} FelicaExtraScene;

extern const NfcProtocolSupportExtraScene felica_extra_scenes[FelicaExtraSceneNum];
