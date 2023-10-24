/**
 * @file nfc_app.h
 * @brief NFC application -- start here.
 *
 * Application for interfacing with NFC cards and other devices via Flipper's built-in NFC hardware.
 *
 * Main features:
 * * Multiple protocols support
 * * Card emulation
 * * Shadow file support
 * * Dynamically loaded parser plugins
 *
 * @see nfc_protocol.h for information on adding a new library protocol.
 * @see nfc_protocol_support.h for information on integrating a library protocol into the app.
 * @see nfc_supported_card_plugin.h for information on adding supported card plugins (parsers).
 */
#pragma once

typedef struct NfcApp NfcApp;
