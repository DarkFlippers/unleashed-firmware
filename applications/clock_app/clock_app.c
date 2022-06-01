#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#define TAG "Clock"

bool timerStarted=false;
int timerSecs=0;
int songSelect=2;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    FuriHalRtcDateTime datetime;
} ClockState;

static void clock_input_callback(InputEvent* input_event, osMessageQueueId_t event_queue) {
    furi_assert(event_queue); 
    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

static void clock_render_callback(Canvas* const canvas, void* ctx) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    ClockState* state = (ClockState*)acquire_mutex((ValueMutex*)ctx, 25);
    char strings[3][20];
    int curMin = (timerSecs/60);
    int curSec = timerSecs-(curMin *60);
    sprintf(strings[0], "%.4d-%.2d-%.2d", state->datetime.year, state->datetime.month, state->datetime.day);
    sprintf(strings[1], "%.2d:%.2d:%.2d", state->datetime.hour, state->datetime.minute, state->datetime.second);
    sprintf(strings[2], "%.2d:%.2d", curMin , curSec);
    release_mutex((ValueMutex*)ctx, state);
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignCenter, strings[1]);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignTop, strings[0]);
    // elements_button_left(canvas, "Alarms");
    // elements_button_right(canvas, "Settings");
    // elements_button_center(canvas, "Reset");
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignTop, strings[2]);
    canvas_set_font(canvas, FontSecondary);
    if(timerStarted) {
        elements_button_center(canvas, "Stop");
    } else {
        elements_button_center(canvas, "Start");
    }
    if(songSelect==0) {
        elements_button_right(canvas, "S:OFF");
    } else if(songSelect==1) {
        elements_button_right(canvas, "S:PoRa");
    } else if(songSelect==2) {
        elements_button_right(canvas, "S:Mario");
    } else if(songSelect==3) {
        elements_button_right(canvas, "S:ByMin");
    }
}

static void clock_state_init(ClockState* const state) {
    furi_hal_rtc_get_datetime(&state->datetime);
}

const NotificationSequence clock_alert_silent = {
    &message_force_vibro_setting_on, &message_vibro_on, &message_red_255, &message_green_255, &message_blue_255, &message_display_backlight_on,
    &message_vibro_off, &message_display_backlight_off, &message_delay_50, &message_display_backlight_on, NULL,
};
const NotificationSequence clock_alert_pr1 = {
    &message_force_speaker_volume_setting_1f,
    &message_force_vibro_setting_on, &message_vibro_on, &message_red_255, &message_green_255, &message_blue_255, &message_display_backlight_on,
    &message_note_g5, &message_delay_100, &message_delay_100, &message_delay_50, &message_sound_off,
    &message_vibro_off, &message_display_backlight_off, &message_delay_50, &message_display_backlight_on,
    &message_note_g5, &message_delay_100, &message_delay_100, &message_delay_50, &message_sound_off, NULL,
};
const NotificationSequence clock_alert_pr2 = {
    &message_force_speaker_volume_setting_1f,
    &message_force_vibro_setting_on, &message_vibro_on,
    &message_note_fs5, &message_delay_100, &message_delay_100, &message_sound_off,
    &message_display_backlight_off, &message_vibro_off, &message_delay_50,
    &message_note_g5, &message_delay_100, &message_delay_100, &message_sound_off,
    &message_display_backlight_on, &message_delay_50,
    &message_note_a5, &message_delay_100, &message_delay_100, &message_sound_off, NULL,
};
const NotificationSequence clock_alert_pr3 = {
    &message_force_speaker_volume_setting_1f,
    &message_display_backlight_off,
    &message_note_g5, &message_delay_100, &message_delay_100, &message_sound_off,
    &message_delay_50, &message_red_255, &message_green_255, &message_blue_255, &message_display_backlight_on, &message_delay_100, NULL,
};
const NotificationSequence clock_alert_mario1 = {
    &message_force_speaker_volume_setting_1f,
    &message_force_vibro_setting_on, &message_vibro_on, &message_red_255, &message_green_255, &message_blue_255, &message_display_backlight_on,
    &message_note_e5, &message_delay_100, &message_delay_100, &message_delay_50, &message_sound_off,
    &message_note_e5, &message_delay_100, &message_delay_100, &message_delay_50, &message_sound_off,
    &message_vibro_off, &message_display_backlight_off, &message_delay_100, &message_display_backlight_on, &message_delay_100,
    &message_note_e5, &message_delay_100, &message_delay_100, &message_delay_50, &message_sound_off, NULL,
};
const NotificationSequence clock_alert_mario2 = {
    &message_force_speaker_volume_setting_1f,
    &message_force_vibro_setting_on, &message_vibro_on, &message_display_backlight_off, &message_delay_100, &message_display_backlight_on, &message_delay_100,
    &message_note_c5, &message_delay_100, &message_delay_100, &message_sound_off,
    &message_display_backlight_off, &message_vibro_off, &message_delay_50,
    &message_note_e5, &message_delay_100, &message_delay_100, &message_sound_off,
    &message_display_backlight_on, NULL,
};
const NotificationSequence clock_alert_mario3 = {
    &message_force_speaker_volume_setting_1f,
    &message_display_backlight_off,
    &message_note_g5, &message_delay_100, &message_delay_100, &message_delay_100, &message_delay_100, &message_sound_off,
    &message_delay_50, &message_red_255, &message_green_255, &message_blue_255, &message_display_backlight_on, &message_delay_100,
    &message_note_g4, &message_delay_100, &message_delay_100, &message_delay_100, &message_delay_100, &message_sound_off,
    NULL,
};
const NotificationSequence clock_alert_perMin = {
    &message_force_speaker_volume_setting_1f,
    &message_note_g5, &message_delay_100, &message_delay_50, &message_sound_off,
    &message_delay_10,
    &message_note_g4, &message_delay_50, &message_delay_10, &message_delay_10, &message_sound_off,
    NULL,
};
const NotificationSequence clock_alert_startStop = {
    &message_force_speaker_volume_setting_1f,
    &message_note_d6, &message_delay_100, &message_delay_10, &message_delay_10, &message_sound_off, NULL,
};

// Runs every 1000ms by default
static void clock_tick(void* ctx) {
    furi_assert(ctx);
    osMessageQueueId_t event_queue = ctx;
    PluginEvent event = {.type = EventTypeTick};
    if(timerStarted) {
        timerSecs=timerSecs+1;
        if(timerSecs%60==0 && songSelect!=0) {
            NotificationApp* notification = furi_record_open("notification");
            notification_message(notification, &clock_alert_perMin);
            furi_record_close("notification");
        }
        if(songSelect==1 ) {
            if(timerSecs==80) {
                NotificationApp* notification = furi_record_open("notification");
                notification_message(notification, &clock_alert_pr1);
                furi_record_close("notification");
            }
            if(timerSecs==81) {
                NotificationApp* notification = furi_record_open("notification");
                notification_message(notification, &clock_alert_pr2);
                furi_record_close("notification");
            }
            if(timerSecs==82) {
                NotificationApp* notification = furi_record_open("notification");
                notification_message(notification, &clock_alert_pr3);
                furi_record_close("notification");
            }
        } else if(songSelect==2 ) {
            if(timerSecs==80) {
                NotificationApp* notification = furi_record_open("notification");
                notification_message(notification, &clock_alert_mario1);
                furi_record_close("notification");
            }
            if(timerSecs==81) {
                NotificationApp* notification = furi_record_open("notification");
                notification_message(notification, &clock_alert_mario2);
                furi_record_close("notification");
            }
            if(timerSecs==82) {
                NotificationApp* notification = furi_record_open("notification");
                notification_message(notification, &clock_alert_mario3);
                furi_record_close("notification");
            }
        } else {
            if(timerSecs==80) {
                NotificationApp* notification = furi_record_open("notification");
                notification_message(notification, &clock_alert_silent);
                furi_record_close("notification");
            }
        }
    }
    // It's OK to loose this event if system overloaded
    osMessageQueuePut(event_queue, &event, 0, 0);
}

int32_t clock_app(void* p) {
    UNUSED(p);
    timerStarted=false;
    timerSecs=0;
    songSelect=2;
    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(PluginEvent), NULL);
    ClockState* plugin_state = malloc(sizeof(ClockState));
    clock_state_init(plugin_state);
    ValueMutex state_mutex;
    if (!init_mutex(&state_mutex, plugin_state, sizeof(ClockState))) {
        FURI_LOG_E(TAG, "cannot create mutex\r\n");
        free(plugin_state);
        return 255;
    }
    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, clock_render_callback, &state_mutex);
    view_port_input_callback_set(view_port, clock_input_callback, event_queue);
    osTimerId_t timer = osTimerNew(clock_tick, osTimerPeriodic, event_queue, NULL);
    osTimerStart(timer, osKernelGetTickFreq());
    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    // Main loop
    PluginEvent event;
    for (bool processing = true; processing;) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 100);
        ClockState* plugin_state = (ClockState*)acquire_mutex_block(&state_mutex);
        if (event_status == osOK) {
            // press events
            if (event.type == EventTypeKey) {
                if (event.input.type == InputTypeShort || event.input.type == InputTypeRepeat) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        if(timerStarted) timerSecs=timerSecs+5;
                        break;
                    case InputKeyDown:
                        if(timerStarted) timerSecs=timerSecs-5;
                        break;
                    case InputKeyRight:
                        if(songSelect==0) {
                            songSelect=1;
                        } else if(songSelect==1)  {
                            songSelect=2;
                        } else if(songSelect==2)  {
                            songSelect=3;
                        } else {
                            songSelect=0;
                        }
                        break;
                    case InputKeyLeft:
                        break;
                    case InputKeyOk: 
                        if(songSelect==1 || songSelect==2 || songSelect==3)  {
                            NotificationApp* notification = furi_record_open("notification");
                            notification_message(notification, &clock_alert_startStop);
                            furi_record_close("notification");
                        }
                        if(timerStarted) {
                            timerStarted=false;
                            timerSecs=0;
                        } else {
                            timerStarted=true;
                        }
                        break;
                    case InputKeyBack:
                        // Exit the plugin
                        processing = false;
                        break;
                    }
                }
            } else if(event.type == EventTypeTick) {
                furi_hal_rtc_get_datetime(&plugin_state->datetime);
            }
        } else {
            FURI_LOG_D(TAG, "osMessageQueue: event timeout");
            // event timeout
        }
        view_port_update(view_port);
        release_mutex(&state_mutex, plugin_state);
    }
    osTimerDelete(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);
    return 0;
}