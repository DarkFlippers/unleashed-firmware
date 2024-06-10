/**
 * @file nfc_protocol.h
 * @brief Top-level NFC protocol definitions.
 *
 * This file is to be modified upon adding a new protocol (see below).
 *
 * # How to add a new NFC protocol
 *
 * ## 1. Gather information
 *
 * Having proper protocol documentation would be ideal, although they are often available only for a fee, or given under an NDA.
 * As an alternative, reading code from relevant open-source projects or notes gathered by means of reverse engineering will do.
 *
 * ### 1.1 Technology
 *
 * Check whether the NFC technology required for the protocol is supported. If no support exists, adding the protocol may
 * be problematic, since it is highly hardware-dependent.
 *
 * @see NfcTech for the enumeration of supported NFC technologies.
 *
 * ### 1.2 Base protocols
 *
 * Check whether the protocol to be implemented is built on top of some already supported protocol.
 *
 * @see NfcProtocol for the enumeration of supported NFC protocols.
 *
 * If the answer is "yes", then the protocol to be implemented is a child protocol. If no, then it will become a base protocol.
 * Sometimes it will be necessary to implement both, e.g. when the target protocol is built on top of a base one,
 * but the latter is currently not supported.
 *
 * ## 2. Create the files
 *
 * ### 2.1 Recommended file structure
 *
 * The recommended file structure for a protocol is as follows:
 *
 * ```text
 * protocols
 *     |
 *     +- protocol_name
 *             |
 *             +- protocol_name.h
 *             |
 *             +- protocol_name.c
 *             |
 *             +- protocol_name_device_defs.h
 *             |
 *             +- protocol_name_poller.h
 *             |
 *             +- protocol_name_poller.c
 *             |
 *             +- protocol_name_poller_defs.h
 *             |
 *             .
 *             . (files below are optional)
 *             .
 *             |
 *             +- protocol_name_listener.h       |
 *             |                                 |
 *             +- protocol_name_listener.c       |- add for emulation support
 *             |                                 |
 *             +- protocol_name_listener_defs.h  |
 *             |
 *             +- protocol_name_sync.h           |
 *             |                                 |- add for synchronous API support
 *             +- protocol_name_sync.c           |
 *             |
 * ```
 *
 * Additionally, an arbitrary amount of private `protocol_name_*_i.h` header files may be created. Do not put implementation
 * details in the regular header files, as they will be exposed in the public firmware API later on.
 *
 * ### 2.2 File structure explanation
 *
 * | Filename                      | Explanation |
 * |:------------------------------|:------------|
 * | protocol_name.h               | Main protocol data structure and associated functions declarations. It is recommended to keep the former as opaque pointer. |
 * | protocol_name.c               | Implementations of functions declared in `protocol_name.h`. |
 * | protocol_name_device_defs.h   | Declarations for use by the NfcDevice library. See nfc_device_base_i.h for more info. |
 * | protocol_name_poller.h        | Protocol-specific poller and associated functions declarations. |
 * | protocol_name_poller.c        | Implementation of functions declared in `protocol_name_poller.h`. |
 * | protocol_name_poller_defs.h   | Declarations for use by the NfcPoller library. See nfc_poller_base.h for more info. |
 * | protocol_name_listener.h      | Protocol-specific listener and associated functions declarations. Optional, needed for emulation support. |
 * | protocol_name_listener.c      | Implementation of functions declared in `protocol_name_listener.h`. Optional, needed for emulation support. |
 * | protocol_name_listener_defs.h | Declarations for use by the NfcListener library. See nfc_listener_base.h for more info. Optional, needed for emulation support. |
 * | protocol_name_sync.h          | Synchronous API declarations. (See below for sync API explanation). Optional.|
 * | protocol_name_sync.c          | Synchronous API implementation. Optional. |
 *
 * ## 3 Implement the code
 *
 * ### 3.1 Protocol data structure
 *
 * A protocol data structure is what holds all data that can be possibly read from a card of a certain type. It may include a unique identifier (UID),
 * configuration bytes and flags, built-in memory, and so on.
 * Additionally, it must implement the NfcDevice interface so that it could be used by the firmware.
 *
 * @see nfc_device_base_i.h for the device interface description.
 *
 * @note It is strongly recommended to implement such a structure as an opaque type and access it via specialised methods only.
 *
 * If the protocol being implemented is a child protocol, then its data must include a pointer to the parent protocol data structure.
 * It is the protocol's responsibility to correctly create and delete the instance the pointer is pointing to.
 *
 * ### 3.2 Poller (reading functionality)
 *
 * A poller contains the functions necessary to successfully read a card of supported type. It must also implement a specific interface,
 * namely described by the NfcPollerBase type.
 *
 * Upon creation, a poller instance will receive either a pointer to the Nfc instance (if it's a base protocol), or a pointer to another poller
 * instance (if it is a child protocol) as the `alloc()` parameter.
 *
 * @see nfc_poller_base.h for the poller interface description.
 *
 * ### 3.3 Listener (emulation functionality)
 *
 * A listener implementation is optional, needed only when emulation is required/possible.
 *
 * Same considerations apply to the listener as for the poller. Additionally, upon creation it will receive an additional parameter
 * in the form of a pointer to the matching protocol data structure, which will be used during emulation. The listener instance
 * does not own this data and does not need to worry about its deletion.
 *
 * @see nfc_listener_base.h for the listener interface description.
 *
 * ### 3.4 Synchronous API
 *
 * Synchronous API does exaclty what it says -- it provides a set of blocking functions for easier use of pollers.
 * Instead of creating all necessary instances and setting up callbacks manually, it does it automatically, at the
 * expense of some flexibility.
 *
 * The most notable use of sync API is in the supported card plugins, so it's a good idea to implement it if such a plugin
 * is to be implemented afterwards.
 *
 * ### 3.5 Registering the protocol
 *
 * After completing the protocol, it must be registered within the NfcProtocol system in order for it to be usable.
 *
 * 1. In nfc_protocol.h, add a new entry in the NfcProtocol enum in the form of NfcProtocolProtocolName.
 * 2. In nfc_protocol.c, add a new entry in the `nfc_protocol_nodes[]` array under the appropriate index.
 *  1. If it is a child protocol, register it as a child in the respective `nfc_base_protocol_name_children_protocol[]` array.
 *  2. If the protocol has children on its own, create a `nfc_protocol_name_children_protocol[]` array
 *     with respective identifiers and register it in the protocol entry added in step 2.
 * 3. In nfc_device_defs.c, include `protocol_name_device_defs.h` and add a pointer to the
 *    `protocol_name_device_defs` structure under the appropriate index.
 * 4. In nfc_poller_defs.c, include `protocol_name_poller_defs.h` and add a pointer to the
 *    `protocol_name_poller_defs` structure under the appropriate index.
 * 5. (Optional) If emulation support was implemented, do the step 4, but for the listener.
 * 6. Add `protocol_name.h`, `protocol_name_poller.h`, and optionally, `protocol_name_listener.h`
 *    and `protocol_name_sync.h` into the `SDK_HEADERS` list in the SConscript file.
 *    This will export the protocol's types and functions for use by the applications.
 * 7. Done!
 *
 * ## What's next?
 *
 * It's about time to integrate the newly implemented protocol into the main NFC application. Without that, reading a card
 * of this type would crash it.
 *
 * @see nfc_protocol_support.h for more information on protocol integration.
 *
 * After having done that, a supported card plugin may be implemented to take further advantage of the new protocol.
 *
 * @see nfc_supported_card_plugin.h for more information on supported card plugins.
 *
 */
#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of all available NFC protocols.
 *
 * When implementing a new protocol, add its identifier before the
 * NfcProtocolNum entry.
 */
typedef enum {
    NfcProtocolIso14443_3a,
    NfcProtocolIso14443_3b,
    NfcProtocolIso14443_4a,
    NfcProtocolIso14443_4b,
    NfcProtocolIso15693_3,
    NfcProtocolFelica,
    NfcProtocolMfUltralight,
    NfcProtocolMfClassic,
    NfcProtocolMfPlus,
    NfcProtocolMfDesfire,
    NfcProtocolSlix,
    NfcProtocolSt25tb,
    /* Add new protocols here */

    NfcProtocolNum, /**< Special value representing the number of available protocols. */

    NfcProtocolInvalid, /**< Special value representing an invalid state. */
} NfcProtocol;

/**
 * @brief Get the immediate parent of a specific protocol.
 *
 * @param[in] protocol identifier of the protocol in question.
 * @returns parent protocol identifier if it has one, or NfcProtocolInvalid otherwise.
 */
NfcProtocol nfc_protocol_get_parent(NfcProtocol protocol);

/**
 * @brief Determine if a specific protocol has a parent on an arbitrary level.
 *
 * Unlike nfc_protocol_get_parent(), this function will traverse the full protocol hierarchy
 * and check each parent node for the matching protocol type.
 *
 * @param[in] protocol identifier of the protocol in question.
 * @param[in] parent_protocol identifier of the parent protocol in question.
 * @returns true if the parent of given type exists, false otherwise.
 */
bool nfc_protocol_has_parent(NfcProtocol protocol, NfcProtocol parent_protocol);

#ifdef __cplusplus
}
#endif
