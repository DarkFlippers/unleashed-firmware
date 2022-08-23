#include "ibutton_protocols.h"
#include "protocol_cyfral.h"
#include "protocol_metakom.h"

const ProtocolBase* ibutton_protocols[] = {
    [iButtonProtocolCyfral] = &protocol_cyfral,
    [iButtonProtocolMetakom] = &protocol_metakom,
};