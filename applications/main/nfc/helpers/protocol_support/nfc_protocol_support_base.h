/**
 * @file nfc_protocol_support_base.h
 * @brief Abstract interface for application-level protocol support.
 */
#pragma once

#include <core/string.h>

#include "../../nfc_app.h"
#include "../../nfc_app_i.h"

/**
 * @brief Scene entry handler.
 *
 * @param[in,out] instance pointer to the NFC application instance.
 */
typedef void (*NfcProtocolSupportOnEnter)(NfcApp* instance);

/**
 * @brief Scene event handler.
 *
 * @param[in,out] instance pointer to the NFC application instance.
 * @param[in] event scene manager event that has occurred.
 * @returns true if the event was handled, false otherwise.
 */
typedef bool (*NfcProtocolSupportOnEvent)(NfcApp* instance, SceneManagerEvent event);

/**
 * @brief Abstract scene interface.
 *
 * on_exit() handler is not implemented due to being redundant.
 */
typedef struct {
    NfcProtocolSupportOnEnter on_enter; /**< Pointer to the on_enter() function. */
    NfcProtocolSupportOnEvent on_event; /**< Pointer to the on_event() function. */
} NfcProtocolSupportSceneBase;

/**
 * @brief Abstract protocol support interface.
 */
typedef struct {
    const uint32_t features; /**< Feature bitmask supported by the protocol. */

    /**
     * @brief Handlers for protocol-specific info scene.
     *
     * This scene displays general information about a saved or recently read card.
     * It may include a button that will lead to more information being shown.
     */
    NfcProtocolSupportSceneBase scene_info;

    /**
     * @brief Handlers for protocol-specific extended info scene.
     *
     * This scene shows more information about a saved or
     * recently read card, such as memory dumps.
     *
     * It may include (a) button(s) and/or menu(s) that will lead to
     * protocol-specific scenes not covered in this helper.
     */
    NfcProtocolSupportSceneBase scene_more_info;

    /**
     * @brief Handlers for protocol-specific read scene.
     *
     * This scene is activated when a read operation is in progress.
     * It is responsible for creating a poller and for handling its events.
     */
    NfcProtocolSupportSceneBase scene_read;

    /**
     * @brief Handlers for protocol-specific read menu scene.
     *
     * This scene presents the user with options available for the
     * recenly read card. Such options may include:
     * * Saving
     * * Getting information
     * * Emulating etc.
     */
    NfcProtocolSupportSceneBase scene_read_menu;

    /**
     * @brief Handlers for protocol-specific read success scene.
     *
     * This scene is activated after a successful read operation.
     * It is responsible for displaying a very short summary about
     * the card that was just read.
     */
    NfcProtocolSupportSceneBase scene_read_success;

    /**
     * @brief Handlers for protocol-specific saved file menu scene.
     *
     * This scene presents the user with options available for a
     * card loaded from file. Such options may include:
     * * Renaming
     * * Deleting
     * * Getting information
     * * Emulating etc.
     */
    NfcProtocolSupportSceneBase scene_saved_menu;

    /**
     * @brief Handlers for protocol-specific name entry scene.
     *
     * This scene is used to enter a file name when saving or renaming a file.
     */
    NfcProtocolSupportSceneBase scene_save_name;

    /**
     * @brief Handlers for protocol-specific emulate scene.
     *
     * This scene is activated when an emulation operation is in progress.
     * It is responsible for creating a listener and for handling its events.
     */
    NfcProtocolSupportSceneBase scene_emulate;
} NfcProtocolSupportBase;
