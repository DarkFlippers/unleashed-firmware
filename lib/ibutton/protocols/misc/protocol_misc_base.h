#pragma once

#include <toolbox/protocols/protocol.h>

#include "../protocol_common_i.h"
#include "../../ibutton_write_targets.h"

#include <one_wire/one_wire_host.h>

/**
 * Write protocol data into a blank of a specific type.
 *
 * Only the protocol knows how its data is laid out in the code frame and what
 * blank it needs, so all the wire-level details stay out of the protocol group.
 *
 * @param host    OneWire host instance.
 * @param target  Blank type to address, one of the protocol's write_targets.
 * @param data    Protocol data, `base.data_size` bytes.
 *
 * @returns true if the blank was written and verified.
 */
typedef bool (*iButtonProtocolMiscWriteIdFunc)(
    OneWireHost* host,
    iButtonWriteTarget target,
    const iButtonProtocolData* data);

typedef struct {
    /** Generic part, used for decoding, encoding and rendering. */
    const ProtocolBase base;

    const uint32_t features;

    /** Blank types a write may try. Non-zero is what advertises WriteId. */
    const iButtonWriteTargetMask write_targets;

    const iButtonProtocolMiscWriteIdFunc write_id;
} iButtonProtocolMiscBase;
