#include "protocol_group_misc_defs.h"

#include "protocol_cyfral.h"
#include "protocol_metakom.h"

const ProtocolBase* ibutton_protocols_misc[] = {
    [iButtonProtocolMiscCyfral] = &ibutton_protocol_misc_cyfral,
    [iButtonProtocolMiscMetakom] = &ibutton_protocol_misc_metakom,
    /* Add new misc protocols here */
};
