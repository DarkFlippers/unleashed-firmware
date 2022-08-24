#pragma once
#include "toolbox/protocols/protocol.h"

typedef enum {
    iButtonProtocolCyfral,
    iButtonProtocolMetakom,

    iButtonProtocolMax,
} iButtonProtocol;

extern const ProtocolBase* ibutton_protocols[];