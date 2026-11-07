#pragma once

#include <toolbox/protocols/protocol.h>

#include "../protocol_common_i.h"

#include <one_wire/one_wire_host.h>

/**
 * Write protocol data into a blank.
 *
 * The protocol chooses the blank writer and hands it the code to program; the
 * writer lays the code out in the frame its blank expects.
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
 * Check whether the protocol can program the data into a blank.
 *
 * Whether the data is valid for reading is a separate question, answered by
 * is_valid().
 *
 * @param data  Protocol data, `base.data_size` bytes.
 *
 * @returns true if the protocol can write the data into a blank.
 */
typedef bool (*iButtonProtocolMiscIsWritableFunc)(const iButtonProtocolData* data);

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

    /** NULL means "no restriction", as for a protocol that writes any data it is given. */
    const iButtonProtocolMiscIsWritableFunc is_writable;

    /** NULL means "nothing to explain", as for a protocol with no redundancy. */
    const iButtonProtocolMiscRenderErrorFunc render_error;
} iButtonProtocolMiscBase;
