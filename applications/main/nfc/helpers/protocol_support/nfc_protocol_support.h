/**
 * @file nfc_protocol_support.h
 * @brief Interface for application-level protocol support.
 *
 * NFC protocol support helper abstracts common scenes with a single interface
 * and lets each protocol decide on concrete implementation.
 *
 * # Integrating a new protocol into the application
 *
 * Most of the scenes in the NFC application work through abstract APIs, so they do not need
 * protocol-specific versions of themselves. However, when such a situation
 * occurs, the protocol support helper provides another level of abstraction to hide
 * the protocol-specific details and isolate them to separate modules.
 *
 * @see nfc_protocol.h for more information on adding library protocols.
 *
 * The steps for adding support for a library protocol are described below.
 *
 * ## 1. Create the files
 *
 * ### 1.1 Recommended file structure
 *
 * The recommended file structure for a protocol support is as follows:
 *
 * ```text
 * protocol_support
 *        |
 *        +- protocol_name
 *                |
 *                +- protocol_name.h
 *                |
 *                +- protocol_name.c
 *                |
 *                +- protocol_name_render.h
 *                |
 *                +- protocol_name_render.c
 *                |
 * ```
 * ### 1.2 File structure explanation
 *
 * | Filename               | Explanation |
 * |:-----------------------|:------------|
 * | protocol_name.h        | Interface structure declaration used in `nfc_protocol_support_defs.c`. |
 * | protocol_name.c        | Protocol-specific scene implemenatations and definitions. |
 * | protocol_name_render.h | Protocol-specific rendering (formatting) functions. Used for converting protocol data into textual descriptions. |
 * | protocol_name_render.c | Implementations for functions declared in `protocol_name_render.h`.|
 *
 * ## 2. Implement the code
 *
 * ### 2.1 Features
 *
 * Decide what features the protocol will be providing. The features can be combined using bitwise OR (`"|"`).
 * This choice influences which scenes will have to be implemented in step 2.2.
 *
 * @see NfcProtocolFeature for the enumeration of possible features to implement.
 *
 * ### 2.2 Scenes
 *
 * If a particular scene is not implemented, its empty placeholder from nfc_protocol_support_gui_common.h must be used instead.
 *
 * @see nfc_protocol_support_common.h for the enumeration of all scenes that can be implemented.
 * @see nfc_protocol_support_base.h for the scene implementation details.
 *
 * ### 2.3. Registering the protocol support
 *
 * After completing the protocol support, it must be registered within the application in order for it to be usable.
 *
 * In nfc_protocol_support_defs.c, include the `protocol_name.h` file and add a new entry in the `nfc_protocol_support[]`
 * array under the appropriate index.
 *
 * ## Done!
 *
 * @note It will not always be possible to abstract all of the protocol's functionality using the protocol support helper.
 * In such cases, creating separate protocol-specific scenes is okay (as an example, note the `nfc/scenes/nfc_scene_mf_classic_*` scenes which didn't fit this paradigm).
 */
#pragma once

#include <gui/scene_manager.h>
#include <lib/nfc/protocols/nfc_protocol.h>

#include "nfc_protocol_support_common.h"

/**
 * @brief Abstract interface for on_enter() scene handler.
 *
 * Is to be called whenever a scene is entered to.
 *
 * @param[in] scene identifier of the scene associated with the handler.
 * @param[in,out] context pointer to a user-specified context (will be passed to concrete handler).
 */
void nfc_protocol_support_on_enter(NfcProtocolSupportScene scene, void* context);

/**
 * @brief Abstract interface for on_event() scene handler.
 *
 * @param[in] scene identifier of the scene associated with the handler.
 * @param[in,out] context pointer to a user-specified context (will be passed to concrete handler).
 * @param[in] event SceneManager event to be handled by the scene.
 * @returns true if the event was consumed, false otherwise.
 */
bool nfc_protocol_support_on_event(
    NfcProtocolSupportScene scene,
    void* context,
    SceneManagerEvent event);

/**
 * @brief Abstract interface for on_exit() scene handler.
 *
 * Is to be called whenever a scene is exited from.
 *
 * @param[in] scene identifier of the scene associated with the handler.
 * @param[in,out] context pointer to a user-specified context (will be passed to concrete handler).
 */
void nfc_protocol_support_on_exit(NfcProtocolSupportScene scene, void* context);

bool nfc_protocol_support_has_feature(NfcProtocol protocol, NfcProtocolFeature feature);
