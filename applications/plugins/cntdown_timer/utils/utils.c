#include <furi.h>
#include "utils.h"

static const NotificationSequence sequence_beep = {
    &message_blue_255,
    &message_note_d5,
    &message_delay_100,
    &message_sound_off,

    NULL,
};

void notification_beep_once() {
    notification_message(furi_record_open(RECORD_NOTIFICATION), &sequence_beep);
    notification_off();
}

void notification_off() {
    furi_record_close(RECORD_NOTIFICATION);
}

void notification_timeup() {
    notification_message(furi_record_open(RECORD_NOTIFICATION), &sequence_audiovisual_alert);
}

void parse_sec_to_time_str(char* buffer, size_t len, int32_t sec) {
    snprintf(
        buffer,
        len,
        "%02ld:%02ld:%02ld",
        (sec % (60 * 60 * 24)) / (60 * 60), // hour
        (sec % (60 * 60)) / 60, // minute
        sec % 60); // second
}
