#include "ibutton_write_targets.h"

#include <furi.h>

#include "protocols/blanks/rw1990.h"
#include "protocols/blanks/tm01x.h"
#include "protocols/blanks/tm2004.h"

// Strictly less than: IBUTTON_WRITE_TARGET_MASK_ALL shifts by iButtonWriteTargetMax, and a
// shift by the full width of the type is undefined.
_Static_assert(
    iButtonWriteTargetMax < 32,
    "A write target mask is a uint32_t, so there is room for 31 targets");

// Saved masks store these as bit positions, so appending is free but reordering silently
// reinterprets every existing settings file - see ibutton_settings.c.
_Static_assert(
    iButtonWriteTargetRW1990_1 == 0 && iButtonWriteTargetRW1990_2 == 1 &&
        iButtonWriteTargetTM2004 == 2 && iButtonWriteTargetTM01x == 3,
    "Saved masks pin these bit positions - append new targets, never reorder");

static const char* const ibutton_write_target_names[iButtonWriteTargetMax] = {
    [iButtonWriteTargetRW1990_1] = "RW1990.1",
    [iButtonWriteTargetRW1990_2] = "RW1990.2",
    [iButtonWriteTargetTM2004] = "TM2004",
    [iButtonWriteTargetTM01x] = "TM01x",
};

// No default: appending a target fails the build until someone decides this, rather than
// silently shipping it enabled. Answering "on" also means bumping IBUTTON_SETTINGS_VERSION,
// or existing files read the new bit as 0 and only fresh installs get it.
static bool ibutton_write_target_is_default(iButtonWriteTarget target) {
    switch(target) {
    case iButtonWriteTargetRW1990_1:
    case iButtonWriteTargetRW1990_2:
    case iButtonWriteTargetTM2004:
    case iButtonWriteTargetTM01x:
        // Each ran unconditionally for the protocols that list it, so all stay on by default.
        return true;
    case iButtonWriteTargetMax:
        break;
    }

    furi_crash("Unknown write target");
}

iButtonWriteTargetMask ibutton_write_targets_default(void) {
    iButtonWriteTargetMask mask = 0;

    for(iButtonWriteTarget target = 0; target < iButtonWriteTargetMax; target++) {
        if(ibutton_write_target_is_default(target)) mask |= IBUTTON_WRITE_TARGET_BIT(target);
    }

    return mask;
}

const char* ibutton_write_target_name(iButtonWriteTarget target) {
    furi_check(target < iButtonWriteTargetMax);

    const char* name = ibutton_write_target_names[target];
    furi_check(name); // catches a target appended without a row above

    return name;
}

bool ibutton_write_target_write(
    OneWireHost* host,
    iButtonWriteTarget target,
    const uint8_t* data,
    size_t data_size) {
    furi_check(host);
    furi_check(data);

    // No default: -Wswitch is an error here, so a target appended to the enum fails the build
    // until it is wired to a writer, rather than silently doing nothing.
    switch(target) {
    case iButtonWriteTargetRW1990_1:
        return rw1990_write_v1(host, data, data_size);
    case iButtonWriteTargetRW1990_2:
        return rw1990_write_v2(host, data, data_size);
    case iButtonWriteTargetTM2004:
        return tm2004_write(host, data, data_size);
    case iButtonWriteTargetTM01x:
        return tm01x_write_dallas(host, data, data_size);
    case iButtonWriteTargetMax:
        break;
    }

    furi_crash("Unknown write target");
}
