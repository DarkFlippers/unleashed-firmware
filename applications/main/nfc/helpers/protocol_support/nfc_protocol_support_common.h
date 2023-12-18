/**
 * @file nfc_protocol_support_common.h
 * @brief Common application-level protocol support definitions.
 */
#pragma once

/**
 * @brief Enumeration of protocol features.
 */
typedef enum {
    NfcProtocolFeatureNone = 0, /**< No features are supported. */
    NfcProtocolFeatureEmulateUid = 1UL << 0, /**< Partial emulation is supported. */
    NfcProtocolFeatureEmulateFull = 1UL << 1, /**< Complete emulation is supported. */
    NfcProtocolFeatureEditUid = 1UL << 2, /**< UID editing is supported. */
    NfcProtocolFeatureMoreInfo = 1UL << 3, /**< More information is provided. */
} NfcProtocolFeature;

/**
 * @brief Enumeration of protocol-aware scenes.
 *
 * These are the scenes that are common to all protocols, but require
 * a protocol-specific implementation.
 */
typedef enum {
    NfcProtocolSupportSceneInfo, /**< Display general card information. */
    NfcProtocolSupportSceneMoreInfo, /**< Display more card information. */
    NfcProtocolSupportSceneRead, /**< Shown when reading a card. */
    NfcProtocolSupportSceneReadMenu, /**< Menu with options available for the recently read card. */
    NfcProtocolSupportSceneReadSuccess, /**< Shown after having successfully read a card. */
    NfcProtocolSupportSceneSavedMenu, /**< Menu for the card that was loaded from file. */
    NfcProtocolSupportSceneSaveName, /**< Shown when saving or renaming a file. */
    NfcProtocolSupportSceneEmulate, /**< Shown when emulating a card. */
    NfcProtocolSupportSceneRpc, /**< Shown in remote-controlled (RPC) mode. */

    NfcProtocolSupportSceneCount, /**< Special value equal to total scene count. Internal use. */
} NfcProtocolSupportScene;
