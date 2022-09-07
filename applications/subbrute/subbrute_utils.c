#include "subbrute_utils.h"

bool subbrute_is_frequency_allowed(SubBruteState* context) {
    // I know you don't like it but laws are laws
    // It's opensource so do whatever you want, but remember the risks :)
    // (Yes, this comment is the only purpose of this function)
    //bool r = furi_hal_region_is_frequency_allowed(context->frequency);
    bool r = true;
    if(!r) {
        FURI_LOG_E(TAG, "Frequency %d is not allowed in your region", context->frequency);
        notification_message(context->notify, &sequence_single_vibro);
    }
    return r;
}