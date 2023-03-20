#include "protocol_group_dallas_defs.h"

#include "protocol_ds1990.h"
#include "protocol_ds1992.h"
#include "protocol_ds1996.h"
#include "protocol_ds1971.h"
#include "protocol_ds_generic.h"

const iButtonProtocolDallasBase* ibutton_protocols_dallas[] = {
    [iButtonProtocolDS1990] = &ibutton_protocol_ds1990,
    [iButtonProtocolDS1992] = &ibutton_protocol_ds1992,
    [iButtonProtocolDS1996] = &ibutton_protocol_ds1996,
    [iButtonProtocolDS1971] = &ibutton_protocol_ds1971,
    /* Add new 1-Wire protocols here */

    /* Default catch-all 1-Wire protocol */
    [iButtonProtocolDSGeneric] = &ibutton_protocol_ds_generic,
};
