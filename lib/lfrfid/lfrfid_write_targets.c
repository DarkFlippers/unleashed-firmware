#include "lfrfid_write_targets.h"

#include <furi.h>
#include <string.h>

_Static_assert(
    LFRFIDWriteTargetMax <= 32,
    "A write target mask is a uint32_t, so there is room for 32 targets");

// Each row states its own variant, so the Hitag targets do not have to sit in any particular
// order. Labels carry "Hitag" where lfrfid_write_target_name() gives the bare variant name,
// which is too cryptic to stand alone in a settings list.
static const struct {
    HitagMicroVariant variant; // only read for LFRFIDWriteTypeHitagMicro targets
    const char* label;
} lfrfid_write_targets[LFRFIDWriteTargetMax] = {
    [LFRFIDWriteTargetT5577] = {0, "T5577"},
    [LFRFIDWriteTargetEM4305] = {0, "EM4305"},
    [LFRFIDWriteTargetHitagMicro8265] = {HitagMicroVariant8265, "Hitag 8265"},
    [LFRFIDWriteTargetHitagMicro8210] = {HitagMicroVariant8210, "Hitag 8210"},
    [LFRFIDWriteTargetHitagMicroH55] = {HitagMicroVariantH55, "Hitag H5.5"},
};

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

    return lfrfid_write_target_label(target);
}

const char* lfrfid_write_target_label(LFRFIDWriteTarget target) {
    furi_check(target < LFRFIDWriteTargetMax);

    const char* label = lfrfid_write_targets[target].label;
    furi_check(label); // a target appended without a row above would land here as NULL

    return label;
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
