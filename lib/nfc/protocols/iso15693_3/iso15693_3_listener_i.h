#pragma once

#include <nfc/protocols/nfc_generic_event.h>

#include "iso15693_3_listener.h"

#include "iso15693_3_i.h"

#ifdef __cplusplus
extern "C" {
#endif

// Based on GET_BLOCKS_SECURITY, one of the commands with lengthier responses:
// - 1 byte flags
// - 1 byte security status * 256 max block count
// - 2 byte crc
// for a response size of 259 bytes.
// There is also READ_MULTI_BLOCKS which has no explicit limit on requested block count
// and ISO 15693-3 also does not specify a maximum overall response length, so this command could
// theoretically result in a 8195 byte response (1 byte flags + 32 byte block * 256 blocks + 2 byte crc);
// for practicality we use a sufficient buffer for a full GET_BLOCKS_SECURITY and
// limit READ_MULTI_BLOCKS to how many blocks we can fit into that buffer size.
#define ISO15693_3_LISTENER_BUFFER_SIZE (259U)

typedef enum {
    Iso15693_3ListenerStateReady,
    Iso15693_3ListenerStateSelected,
    Iso15693_3ListenerStateQuiet,
} Iso15693_3ListenerState;

typedef struct {
    bool selected;
    bool addressed;
    bool wait_for_eof;
} Iso15693_3ListenerSessionState;

typedef Iso15693_3Error (*Iso15693_3ExtensionHandler)(void* context, va_list args);

typedef struct {
    Iso15693_3ExtensionHandler mandatory[ISO15693_3_MANDATORY_COUNT];
    Iso15693_3ExtensionHandler optional[ISO15693_3_OPTIONAL_COUNT];
} Iso15693_3ExtensionHandlerTable;

struct Iso15693_3Listener {
    Nfc* nfc;
    Iso15693_3Data* data;
    Iso15693_3ListenerState state;
    Iso15693_3ListenerSessionState session_state;
    BitBuffer* tx_buffer;

    NfcGenericEvent generic_event;
    Iso15693_3ListenerEvent iso15693_3_event;
    Iso15693_3ListenerEventData iso15693_3_event_data;
    NfcGenericCallback callback;
    void* context;

    const Iso15693_3ExtensionHandlerTable* extension_table;
    void* extension_context;
};

Iso15693_3Error iso15693_3_listener_set_extension_handler_table(
    Iso15693_3Listener* instance,
    const Iso15693_3ExtensionHandlerTable* table,
    void* context);

Iso15693_3Error iso15693_3_listener_ready(Iso15693_3Listener* instance);

Iso15693_3Error
    iso15693_3_listener_send_frame(Iso15693_3Listener* instance, const BitBuffer* tx_buffer);

Iso15693_3Error
    iso15693_3_listener_process_request(Iso15693_3Listener* instance, const BitBuffer* rx_buffer);

Iso15693_3Error iso15693_3_listener_process_single_eof(Iso15693_3Listener* instance);

Iso15693_3Error iso15693_3_listener_process_uid_mismatch(
    Iso15693_3Listener* instance,
    const BitBuffer* rx_buffer);

#ifdef __cplusplus
}
#endif
