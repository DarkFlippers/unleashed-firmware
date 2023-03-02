#include "protocol_group_defs.h"

#include "dallas/protocol_group_dallas.h"
#include "misc/protocol_group_misc.h"

const iButtonProtocolGroupBase* ibutton_protocol_groups[] = {
    [iButtonProtocolGroupDallas] = &ibutton_protocol_group_dallas,
    [iButtonProtocolGroupMisc] = &ibutton_protocol_group_misc,
};
