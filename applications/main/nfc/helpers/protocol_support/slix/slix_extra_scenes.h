/**
 * @file slix_extra_scenes.h
 * @brief Indices of the Slix scenes that live in the plugin.
 *
 * Shared between the thunks in the app (scenes/nfc_scene_slix_*.c), which name the index, and
 * the plugin, which supplies the handlers.
 */
#pragma once

#include "../nfc_protocol_support_base.h"

typedef enum {
    SlixExtraSceneUnlockMenu,
    SlixExtraSceneKeyInput,
    SlixExtraSceneUnlock,
    SlixExtraSceneUnlockSuccess,

    SlixExtraSceneNum,
} SlixExtraScene;

extern const NfcProtocolSupportExtraScene slix_extra_scenes[SlixExtraSceneNum];
