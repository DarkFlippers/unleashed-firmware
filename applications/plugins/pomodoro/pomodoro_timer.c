#include "pomodoro_timer.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <notification/notification_messages.h>
#include <Pomodoro_Timer_icons.h>

const NotificationSequence sequence_finish = {
    &message_display_backlight_on,
    &message_green_255,
    &message_vibro_on,
    &message_note_c5,
    &message_delay_100,
    &message_vibro_off,
    &message_vibro_on,
    &message_note_e5,
    &message_delay_100,
    &message_vibro_off,
    &message_vibro_on,
    &message_note_g5,
    &message_delay_100,
    &message_vibro_off,
    &message_vibro_on,
    &message_note_b5,
    &message_delay_250,
    &message_vibro_off,
    &message_vibro_on,
    &message_note_c6,
    &message_delay_250,
    &message_vibro_off,
    &message_sound_off,
    NULL,
};

const NotificationSequence sequence_rest = {
    &message_display_backlight_on,
    &message_red_255,
    &message_vibro_on,
    &message_note_c6,
    &message_delay_100,
    &message_vibro_off,
    &message_vibro_on,
    &message_note_b5,
    &message_delay_100,
    &message_vibro_off,
    &message_vibro_on,
    &message_note_g5,
    &message_delay_100,
    &message_vibro_off,
    &message_vibro_on,
    &message_note_e5,
    &message_delay_100,
    &message_vibro_off,
    &message_vibro_on,
    &message_note_c5,
    &message_delay_250,
    &message_vibro_off,
    &message_sound_off,
    NULL,
};

void pomodoro_timer_process(PomodoroTimer* pomodoro_timer, InputEvent* event) {
    with_view_model(
        pomodoro_timer->view,
        PomodoroTimerModel * model,
        {
            if(event->type == InputTypePress) {
                if(event->key == InputKeyOk) {
                    model->ok_pressed = true;
                } else if(event->key == InputKeyLeft) {
                    model->reset_pressed = true;
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = true;
                }
            } else if(event->type == InputTypeRelease) {
                if(event->key == InputKeyOk) {
                    model->ok_pressed = false;

                    // START/STOP TIMER
                    FuriHalRtcDateTime curr_dt;
                    furi_hal_rtc_get_datetime(&curr_dt);
                    uint32_t current_timestamp = furi_hal_rtc_datetime_to_timestamp(&curr_dt);

                    // STARTED -> PAUSED
                    if(model->timer_running) {
                        // Update stopped seconds
                        model->timer_stopped_seconds =
                            current_timestamp - model->timer_start_timestamp;
                    } else if(!model->time_passed) {
                        // INITIAL -> STARTED
                        model->timer_start_timestamp = current_timestamp;
                        model->rest_running = false;
                    } else {
                        // PAUSED -> STARTED
                        model->timer_start_timestamp =
                            current_timestamp - model->timer_stopped_seconds;
                    }
                    model->timer_running = !model->timer_running;
                } else if(event->key == InputKeyLeft) {
                    if(!model->timer_running) {
                        furi_record_close(RECORD_NOTIFICATION);
                        model->timer_stopped_seconds = 0;
                        model->timer_start_timestamp = 0;
                        model->time_passed = 0;
                        model->timer_running = false;
                    }
                    model->reset_pressed = false;
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = false;
                }
            }
        },
        true);
}

void pomodoro_draw_callback(Canvas* canvas, void* context, int max_seconds, int max_seconds_rest) {
    furi_assert(context);
    PomodoroTimerModel* model = context;
    FuriHalRtcDateTime curr_dt;
    furi_hal_rtc_get_datetime(&curr_dt);
    uint32_t current_timestamp = furi_hal_rtc_datetime_to_timestamp(&curr_dt);

    // Header
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 0, 0, AlignLeft, AlignTop, "Pomodoro");

    canvas_draw_icon(canvas, 68, 1, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 127, 1, AlignRight, AlignTop, "Hold to exit");

    // Start/Pause/Continue
    int txt_main_y = 34;
    canvas_draw_icon(canvas, 63, 23, &I_Space_65x18); // button
    if(model->ok_pressed) {
        elements_slightly_rounded_box(canvas, 66, 25, 60, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    if(model->timer_running) {
        model->time_passed = current_timestamp - model->timer_start_timestamp;
        elements_multiline_text_aligned(canvas, 83, txt_main_y, AlignLeft, AlignBottom, "Pause");
        canvas_draw_box(canvas, 71, 27, 2, 8);
        canvas_draw_box(canvas, 75, 27, 2, 8);
    } else {
        if(model->time_passed) {
            elements_multiline_text_aligned(
                canvas, 83, txt_main_y, AlignLeft, AlignBottom, "Continue");
        } else {
            elements_multiline_text_aligned(
                canvas, 83, txt_main_y, AlignLeft, AlignBottom, "Start");
        }
        canvas_draw_icon(canvas, 70, 26, &I_Ok_btn_9x9); // OK icon
    }
    canvas_set_color(canvas, ColorBlack);

    // Reset
    if(!model->timer_running && model->time_passed) {
        canvas_draw_icon(canvas, 63, 46, &I_Space_65x18);
        if(model->reset_pressed) {
            elements_slightly_rounded_box(canvas, 66, 48, 60, 13);
            canvas_set_color(canvas, ColorWhite);
        }
        canvas_draw_icon(canvas, 72, 50, &I_ButtonLeft_4x7);
        elements_multiline_text_aligned(canvas, 83, 57, AlignLeft, AlignBottom, "Reset");
        canvas_set_color(canvas, ColorBlack);
    }

    char buffer[64];

    // Time to work
    int total_time_left = (max_seconds - (uint32_t)model->time_passed);
    int minutes_left = total_time_left / 60;
    int seconds_left = total_time_left % 60;
    canvas_set_font(canvas, FontBigNumbers);

    // Play sound
    if(total_time_left == 0 && !model->sound_playing) {
        model->sound_playing = true;
        notification_message(furi_record_open(RECORD_NOTIFICATION), &sequence_finish);
    }
    if(total_time_left < 0) {
        model->timer_running = false;
        model->time_passed = 0;
        model->sound_playing = false;

        model->rest_running = true;
        model->rest_start_timestamp = current_timestamp;
        seconds_left = 0;
        model->counter += 1;
    }
    if(!model->rest_running) {
        snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes_left, seconds_left);
        canvas_draw_str(canvas, 0, 39, buffer);
    }
    if(model->timer_running) {
        canvas_set_font(canvas, FontPrimary);
        elements_multiline_text_aligned(canvas, 0, 50, AlignLeft, AlignTop, "Time to work");
    }

    // Time to rest
    if(model->rest_running && !model->timer_running) {
        canvas_set_font(canvas, FontBigNumbers);
        int rest_passed = current_timestamp - model->rest_start_timestamp;
        int rest_total_time_left = (max_seconds_rest - rest_passed);
        int rest_minutes_left = rest_total_time_left / 60;
        int rest_seconds_left = rest_total_time_left % 60;

        // Play sound
        if(rest_total_time_left == 0 && !model->sound_playing) {
            model->sound_playing = true;
            notification_message(furi_record_open(RECORD_NOTIFICATION), &sequence_rest);
        }
        if(rest_total_time_left < 0) {
            rest_seconds_left = 0;
            model->rest_running = false;
            model->sound_playing = false;
        }
        snprintf(buffer, sizeof(buffer), "%02d:%02d", rest_minutes_left, rest_seconds_left);
        canvas_draw_str(canvas, 0, 60, buffer);

        canvas_set_font(canvas, FontPrimary);
        elements_multiline_text_aligned(canvas, 0, 27, AlignLeft, AlignTop, "Have a rest");
    }

    // Clocks
    canvas_set_font(canvas, FontSecondary);
    snprintf(
        buffer,
        sizeof(buffer),
        "%02ld:%02ld:%02ld",
        ((uint32_t)current_timestamp % (60 * 60 * 24)) / (60 * 60),
        ((uint32_t)current_timestamp % (60 * 60)) / 60,
        (uint32_t)current_timestamp % 60);
    canvas_draw_str(canvas, 0, 20, buffer);

    // Tomato counter
    if(model->counter > 5) {
        model->counter = 1;
    }
    for(int i = 0; i < model->counter; ++i) {
        canvas_draw_disc(canvas, 122 - i * 10, 15, 4);
    }
}
