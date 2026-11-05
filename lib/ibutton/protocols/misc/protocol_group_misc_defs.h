#pragma once

#include "protocol_misc_base.h"

typedef enum {
    iButtonProtocolMiscCyfral,
    iButtonProtocolMiscMetakom,
    iButtonProtocolMiscMax,
} iButtonProtocolMisc;

/** Full protocol descriptions, including write support. */
extern const iButtonProtocolMiscBase* const ibutton_protocols_misc[];

/** Generic parts only, for the shared decoder/encoder/render machinery. */
extern const ProtocolBase* const ibutton_protocols_misc_dict[];
