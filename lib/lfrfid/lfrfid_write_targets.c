#include "lfrfid_write_targets.h"

#include <furi.h>
#include <string.h>

// Strictly less than: LFRFID_WRITE_TARGET_MASK_ALL shifts by LFRFIDWriteTargetMax, and a shift
// by the full width of the type is undefined.
_Static_assert(
    LFRFIDWriteTargetMax < 32,
    "A write target mask is a uint32_t, so there is room for 31 targets");

// Each row states its own variant, so the Hitag micro targets do not have to sit in any
// particular order. Their names are left NULL because hitagmicro.c already owns those strings,
// and a second copy here is a second thing to keep in step.
static const struct {
    HitagMicroVariant variant; // only read for LFRFIDWriteTypeHitagMicro targets
    const char* name;
} lfrfid_write_targets[LFRFIDWriteTargetMax] = {
    [LFRFIDWriteTargetT5577] = {0, "T5577"},
    [LFRFIDWriteTargetEM4305] = {0, "EM4305"},
    [LFRFIDWriteTargetHitagMicro8265] = {HitagMicroVariant8265, NULL},
    [LFRFIDWriteTargetHitagMicro8210] = {HitagMicroVariant8210, NULL},
    [LFRFIDWriteTargetHitagMicroH55] = {HitagMicroVariantH55, NULL},
    [LFRFIDWriteTargetHitagS8268] = {.name = "8268"},
};

/** Is this target safe to try without the user having asked for it?
 *
 * No default, for the same reason lfrfid_write_target_type() has none: a target appended to the
 * enum fails the build until someone decides this, rather than silently shipping enabled. Getting
 * the encoding wrong shows up on the first test write; getting this wrong ships a destructive
 * write to everyone who never opened Settings.
 */
static bool lfrfid_write_target_is_default(LFRFIDWriteTarget target) {
    switch(target) {
    case LFRFIDWriteTargetT5577:
    case LFRFIDWriteTargetEM4305:
    case LFRFIDWriteTargetHitagMicro8265:
    case LFRFIDWriteTargetHitagMicro8210:
    case LFRFIDWriteTargetHitagMicroH55:
        // Blind writers, and their chips are blanks or clones: a write that lands on the wrong
        // card does nothing to it.
        return true;
    case LFRFIDWriteTargetHitagS8268:
        // Pages 4 and 5 hold application data on a genuine Hitag S, nothing this hardware can read
        // tells a genuine tag from a clone, and unlike the blind writers this one selects whatever
        // tag answers - so it must not be something a user runs into by cloning a key.
        return false;
    case LFRFIDWriteTargetMax:
        break;
    }

    furi_crash("Unknown write target");
}

LFRFIDWriteTargetMask lfrfid_write_targets_default(void) {
    LFRFIDWriteTargetMask mask = 0;

    for(LFRFIDWriteTarget target = 0; target < LFRFIDWriteTargetMax; target++) {
        if(lfrfid_write_target_is_default(target)) mask |= LFRFID_WRITE_TARGET_BIT(target);
    }

    return mask;
}

LFRFIDWriteType lfrfid_write_target_type(LFRFIDWriteTarget target) {
    // No default: -Wswitch is an error here, so a target appended to the enum fails the build
    // until it is classified, rather than silently inheriting another chip's encoding.
    switch(target) {
    case LFRFIDWriteTargetT5577:
        return LFRFIDWriteTypeT5577;
    case LFRFIDWriteTargetEM4305:
        return LFRFIDWriteTypeEM4305;
    case LFRFIDWriteTargetHitagMicro8265:
    case LFRFIDWriteTargetHitagMicro8210:
    case LFRFIDWriteTargetHitagMicroH55:
        return LFRFIDWriteTypeHitagMicro;
    case LFRFIDWriteTargetHitagS8268:
        return LFRFIDWriteTypeHitagS;
    case LFRFIDWriteTargetMax:
        break;
    }

    furi_crash("Unknown write target");
}

HitagMicroVariant lfrfid_write_target_variant(LFRFIDWriteTarget target) {
    furi_check(lfrfid_write_target_type(target) == LFRFIDWriteTypeHitagMicro);

    return lfrfid_write_targets[target].variant;
}

const char* lfrfid_write_target_name(LFRFIDWriteTarget target) {
    if(lfrfid_write_target_type(target) == LFRFIDWriteTypeHitagMicro) {
        return hitagmicro_variant_name(lfrfid_write_target_variant(target));
    }

    // Catches a target appended without a row above. A Hitag micro one cannot be caught here:
    // its row reads back as variant 0, which is a real variant (8265).
    const char* name = lfrfid_write_targets[target].name;
    furi_check(name);

    return name;
}

LFRFIDWriteTargetMask lfrfid_write_targets_supported(ProtocolDict* dict, ProtocolId protocol) {
    furi_check(dict);
    furi_check(protocol != PROTOCOL_NO);

    // Heap, not stack: LFRFIDWriteRequest is ~70 bytes and callers may be on a 2K stack.
    LFRFIDWriteRequest* request = malloc(sizeof(LFRFIDWriteRequest));
    LFRFIDWriteTargetMask mask = 0;

    for(LFRFIDWriteTarget target = 0; target < LFRFIDWriteTargetMax; target++) {
        memset(request, 0, sizeof(LFRFIDWriteRequest));
        request->write_type = lfrfid_write_target_type(target);

        if(protocol_dict_get_write_data(dict, protocol, request)) {
            mask |= LFRFID_WRITE_TARGET_BIT(target);
        }
    }

    free(request);

    return mask;
}
