#include "camera_suite_speaker.h"
#include "../camera_suite.h"

#define NOTE_INPUT 587.33f

void camera_suite_play_input_sound(void* context) {
    CameraSuite* app = context;
    if(app->speaker != 1) {
        return;
    }
    float volume = 1.0f;
    if(furi_hal_speaker_is_mine() || furi_hal_speaker_acquire(30)) {
        furi_hal_speaker_start(NOTE_INPUT, volume);
    }
}

void camera_suite_stop_all_sound(void* context) {
    CameraSuite* app = context;
    if(app->speaker != 1) {
        return;
    }
    if(furi_hal_speaker_is_mine()) {
        furi_hal_speaker_stop();
        furi_hal_speaker_release();
    }
}
