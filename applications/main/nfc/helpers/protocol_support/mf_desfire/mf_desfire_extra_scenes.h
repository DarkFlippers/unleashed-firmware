/**
 * @file mf_desfire_extra_scenes.h
 * @brief Indices of the MfDesfire scenes that live in the plugin.
 *
 * Shared between the thunks in the app (scenes/nfc_scene_mf_desfire_*.c), which name the index, and
 * the plugin, which supplies the handlers.
 */
#pragma once

#include "../nfc_protocol_support_base.h"

typedef enum {
    MfDesfireExtraSceneMoreInfo,
    MfDesfireExtraSceneApp,

    MfDesfireExtraSceneNum,
} MfDesfireExtraScene;

extern const NfcProtocolSupportExtraScene mf_desfire_extra_scenes[MfDesfireExtraSceneNum];
