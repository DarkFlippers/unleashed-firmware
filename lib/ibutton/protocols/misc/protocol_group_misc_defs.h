#pragma once

#include <toolbox/protocols/protocol.h>

typedef enum {
    iButtonProtocolMiscCyfral,
    iButtonProtocolMiscMetakom,
    iButtonProtocolMiscMax,
} iButtonProtocolMisc;

extern const ProtocolBase* const ibutton_protocols_misc[];
