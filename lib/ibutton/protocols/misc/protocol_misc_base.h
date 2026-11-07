#pragma once

#include <toolbox/protocols/protocol.h>

#include "../protocol_common_i.h"

#include <one_wire/one_wire_host.h>

/**
 * Write protocol data into a blank.
 *
 * Only the protocol knows how its data is laid out in the code frame and what
 * blank it needs, so all the wire-level details stay out of the protocol group.
 *
 * @param host  OneWire host instance.
 * @param data  Protocol data, `base.data_size` bytes.
 *
 * @returns true if the blank was written and verified.
 */
typedef bool (*iButtonProtocolMiscWriteIdFunc)(OneWireHost* host, const iButtonProtocolData* data);

/**
 * Check the protocol's own redundancy in a key's data.
 *
 * @param data  Protocol data, `base.data_size` bytes.
 *
 * @returns true if the data is a code the protocol could have produced.
 */
typedef bool (*iButtonProtocolMiscIsValidFunc)(const iButtonProtocolData* data);

/**
 * Explain to the user why is_valid() rejected the data.
 *
 * @param result  String to append the explanation to.
 * @param data    Protocol data, `base.data_size` bytes.
 */
typedef void (
    *iButtonProtocolMiscRenderErrorFunc)(FuriString* result, const iButtonProtocolData* data);

typedef struct {
    /** Generic part, used for decoding, encoding and rendering. */
    const ProtocolBase base;

    const iButtonProtocolMiscWriteIdFunc write_id;

    /** NULL means "every value is valid", as for a protocol with no redundancy. */
    const iButtonProtocolMiscIsValidFunc is_valid;

    /** NULL means "nothing to explain", as for a protocol with no redundancy. */
    const iButtonProtocolMiscRenderErrorFunc render_error;
} iButtonProtocolMiscBase;
