/**
 * @file emv_extra_scenes.h
 * @brief Indices of the Emv scenes that live in the plugin.
 *
 * Shared between the thunks in the app (scenes/nfc_scene_emv_*.c), which name the index, and
 * the plugin, which supplies the handlers.
 */
#pragma once

#include "../nfc_protocol_support_base.h"

typedef enum {
    EmvExtraSceneTransactions,

    EmvExtraSceneNum,
} EmvExtraScene;

extern const NfcProtocolSupportExtraScene emv_extra_scenes[EmvExtraSceneNum];
