/**
 * @file mf_classic_extra_scenes.h
 * @brief Indices of the MfClassic scenes that live in the plugin.
 *
 * Shared between the thunks in the app (scenes/nfc_scene_mf_classic_*.c), which name the index, and
 * the plugin, which supplies the handlers.
 */
#pragma once

#include "../nfc_protocol_support_base.h"

typedef enum {
    MfClassicExtraSceneDictAttack,
    MfClassicExtraSceneDetectReader,
    MfClassicExtraSceneMfkeyComplete,
    MfClassicExtraSceneMfkeyNoncesInfo,
    MfClassicExtraSceneShowKeys,
    MfClassicExtraSceneUpdateInitial,

    MfClassicExtraSceneNum,
} MfClassicExtraScene;

extern const NfcProtocolSupportExtraScene mf_classic_extra_scenes[MfClassicExtraSceneNum];
