/**
 * @file mf_plus_extra_scenes.h
 * @brief Indices of the MfPlus scenes that live in the plugin.
 *
 * Shared between the thunks in the app (scenes/nfc_scene_mf_plus_*.c), which name the index, and
 * the plugin, which supplies the handlers.
 */
#pragma once

#include "../nfc_protocol_support_base.h"

typedef enum {
    MfPlusExtraSceneDictAttack,
    MfPlusExtraSceneShowKeys,
    MfPlusExtraSceneMoreInfo,
    MfPlusExtraSceneIso4Info,
    MfPlusExtraSceneVersion,
    MfPlusExtraSceneUpdateInitial,

    MfPlusExtraSceneNum,
} MfPlusExtraScene;

extern const NfcProtocolSupportExtraScene mf_plus_extra_scenes[MfPlusExtraSceneNum];
