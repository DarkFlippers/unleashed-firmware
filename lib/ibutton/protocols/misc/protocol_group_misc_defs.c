#include "protocol_group_misc_defs.h"

#include <core/core_defines.h>

#include "protocol_cyfral.h"
#include "protocol_metakom.h"

const iButtonProtocolMiscBase* const ibutton_protocols_misc[] = {
    [iButtonProtocolMiscCyfral] = &ibutton_protocol_misc_cyfral,
    [iButtonProtocolMiscMetakom] = &ibutton_protocol_misc_metakom,
    /* Add new misc protocols here */
};

const ProtocolBase* const ibutton_protocols_misc_dict[] = {
    [iButtonProtocolMiscCyfral] = &ibutton_protocol_misc_cyfral.base,
    [iButtonProtocolMiscMetakom] = &ibutton_protocol_misc_metakom.base,
    /* Add new misc protocols here */
};

_Static_assert(
    COUNT_OF(ibutton_protocols_misc) == iButtonProtocolMiscMax,
    "Every iButtonProtocolMisc needs a row in ibutton_protocols_misc[]");
_Static_assert(
    COUNT_OF(ibutton_protocols_misc_dict) == iButtonProtocolMiscMax,
    "Every iButtonProtocolMisc needs a row in ibutton_protocols_misc_dict[]");
