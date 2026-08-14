/**
 * @file mf_ultralight_extra_scenes.h
 * @brief Indices of the MIFARE Ultralight scenes that live in the plugin.
 *
 * Shared between the thunks in the app (scenes/nfc_scene_mf_ultralight_*.c), which name the index,
 * and the plugin, which supplies the handlers.
 */
#pragma once

#include "../nfc_protocol_support_base.h"

typedef enum {
    MfUltralightExtraSceneCDictAttack,
    MfUltralightExtraSceneAesDictAttack,
    MfUltralightExtraSceneUnlockMenu,
    MfUltralightExtraSceneUnlockWarn,
    MfUltralightExtraSceneKeyInput,
    MfUltralightExtraSceneCapturePass,

    MfUltralightExtraSceneNum,
} MfUltralightExtraScene;

extern const NfcProtocolSupportExtraScene mf_ultralight_extra_scenes[MfUltralightExtraSceneNum];
