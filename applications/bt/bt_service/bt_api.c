#include "bt_i.h"

bool bt_set_profile(Bt* bt, BtProfile profile) {
    furi_assert(bt);

    // Send message
    bool result = false;
    BtMessage message = {
        .type = BtMessageTypeSetProfile, .data.profile = profile, .result = &result};
    furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
    // Wait for unlock
    osEventFlagsWait(bt->api_event, BT_API_UNLOCK_EVENT, osFlagsWaitAny, osWaitForever);

    return result;
}
