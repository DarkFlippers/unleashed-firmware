#include "bt.h"
#include "bt_i.h"

void bt_update_battery_level(Bt* bt, uint8_t battery_level) {
    furi_assert(bt);
    BtMessage message = {
        .type = BtMessageTypeUpdateBatteryLevel, .data.battery_level = battery_level};
    furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
}

bool bt_pin_code_show(Bt* bt, uint32_t pin_code) {
    furi_assert(bt);
    BtMessage message = {.type = BtMessageTypePinCodeShow, .data.pin_code = pin_code};
    return osMessageQueuePut(bt->message_queue, &message, 0, 0) == osOK;
}
