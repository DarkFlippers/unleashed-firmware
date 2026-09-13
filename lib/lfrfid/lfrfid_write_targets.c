#include "lfrfid_write_targets.h"

#include <furi.h>
#include <string.h>

// A new Hitag micro variant has to be added to LFRFIDWriteTarget as well, or its targets and
// the saved settings mask silently stop lining up with HitagMicroVariant.
_Static_assert(
    LFRFIDWriteTargetMax == LFRFIDWriteTargetHitagMicro8265 + HitagMicroVariantCount,
    "Hitag micro variants and write targets are out of sync");

// Only the Hitag entries differ from lfrfid_write_target_name(): the bare variant names are
// too cryptic to stand alone in a settings list.
static const char* const lfrfid_write_target_labels[LFRFIDWriteTargetMax] = {
    [LFRFIDWriteTargetT5577] = "T5577",
    [LFRFIDWriteTargetEM4305] = "EM4305",
    [LFRFIDWriteTargetHitagMicro8265] = "Hitag 8265",
    [LFRFIDWriteTargetHitagMicro8210] = "Hitag 8210",
    [LFRFIDWriteTargetHitagMicroH55] = "Hitag H5.5",
};

LFRFIDWriteType lfrfid_write_target_type(LFRFIDWriteTarget target) {
    furi_check(target < LFRFIDWriteTargetMax);

    if(target == LFRFIDWriteTargetT5577) return LFRFIDWriteTypeT5577;
    if(target == LFRFIDWriteTargetEM4305) return LFRFIDWriteTypeEM4305;
    return LFRFIDWriteTypeHitagMicro;
}

HitagMicroVariant lfrfid_write_target_variant(LFRFIDWriteTarget target) {
    furi_check(target >= LFRFIDWriteTargetHitagMicro8265);
    furi_check(target < LFRFIDWriteTargetMax);

    return (HitagMicroVariant)(target - LFRFIDWriteTargetHitagMicro8265);
}

const char* lfrfid_write_target_name(LFRFIDWriteTarget target) {
    furi_check(target < LFRFIDWriteTargetMax);

    if(target == LFRFIDWriteTargetT5577) return "T5577";
    if(target == LFRFIDWriteTargetEM4305) return "EM4305";
    return hitagmicro_variant_name(lfrfid_write_target_variant(target));
}

const char* lfrfid_write_target_label(LFRFIDWriteTarget target) {
    furi_check(target < LFRFIDWriteTargetMax);

    return lfrfid_write_target_labels[target];
}

uint32_t lfrfid_write_targets_supported(ProtocolDict* dict, ProtocolId protocol) {
    furi_check(dict);
    furi_check(protocol != PROTOCOL_NO);

    // Heap, not stack: the request is ~80 bytes and both callers run on a 2K thread stack.
    LFRFIDWriteRequest* request = malloc(sizeof(LFRFIDWriteRequest));
    uint32_t mask = 0;

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
