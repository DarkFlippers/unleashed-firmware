#include <furi.h>
#include <gui/gui.h>
#include <api-hal-version.h>
#include "dolphin/dolphin.h"
#include "dolphin/dolphin_state.h"
#include "math.h"

#define MOODS_TOTAL 3
#define BUTTHURT_MAX 3

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    union {
        InputEvent input;
    } value;
    EventType type;
} AppEvent;

// Moods, corresponding to butthurt level. (temp, unclear about max level)
static const char* mood_strings[MOODS_TOTAL] = {[0] = "Happy", [1] = "Ok", [2] = "Bad"};

static const Icon* portrait_happy[BUTTHURT_MAX] = {
    &I_passport_happy1_43x45,
    &I_passport_happy2_43x45,
    &I_passport_happy3_43x45};
static const Icon* portrait_ok[BUTTHURT_MAX] = {
    &I_passport_okay1_43x45,
    &I_passport_okay2_43x45,
    &I_passport_okay3_43x45};
static const Icon* portrait_bad[BUTTHURT_MAX] = {
    &I_passport_bad1_43x45,
    &I_passport_bad2_43x45,
    &I_passport_bad3_43x45};
static const Icon** portraits[MOODS_TOTAL] = {portrait_happy, portrait_ok, portrait_bad};

static void input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = ctx;
    AppEvent event;
    event.type = EventTypeKey;
    event.value.input = *input_event;
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

static void render_callback(Canvas* canvas, void* ctx) {
    DolphinState* state = (DolphinState*)acquire_mutex((ValueMutex*)ctx, 25);

    char level[20];
    char mood[32];

    uint32_t butthurt = CLAMP(dolphin_state_get_butthurt(state), BUTTHURT_MAX - 1, 0);
    uint32_t current_level = dolphin_state_get_level(state);
    uint32_t prev_cap = dolphin_state_xp_to_levelup(state, current_level - 1, false);
    uint32_t exp = (dolphin_state_xp_to_levelup(state, current_level, true) * 63) /
                   (dolphin_state_xp_to_levelup(state, current_level, false) - prev_cap);
    uint8_t portrait_level = CLAMP(floor(current_level / 14), MOODS_TOTAL - 1, 0);

    canvas_clear(canvas);

    // multipass
    canvas_draw_icon(canvas, 0, 0, &I_PassportLeft_6x47);
    canvas_draw_icon(canvas, 0, 47, &I_PassportBottom_128x17);
    canvas_draw_line(canvas, 6, 0, 125, 0);
    canvas_draw_line(canvas, 127, 2, 127, 47);
    canvas_draw_dot(canvas, 126, 1);

    //portrait frame
    canvas_draw_line(canvas, 9, 6, 9, 53);
    canvas_draw_line(canvas, 10, 5, 52, 5);
    canvas_draw_line(canvas, 55, 8, 55, 53);
    canvas_draw_line(canvas, 10, 54, 54, 54);
    canvas_draw_line(canvas, 53, 5, 55, 7);

    // portrait
    canvas_draw_icon(canvas, 10, 9, portraits[butthurt][portrait_level]);
    canvas_draw_line(canvas, 59, 18, 124, 18);
    canvas_draw_line(canvas, 59, 31, 124, 31);
    canvas_draw_line(canvas, 59, 44, 124, 44);

    const char* my_name = api_hal_version_get_name_ptr();
    canvas_draw_str(canvas, 59, 15, my_name ? my_name : "Unknown");

    snprintf(level, 20, "Level: %ld", current_level);
    snprintf(mood, 20, "Mood: %s", mood_strings[butthurt]);

    canvas_draw_str(canvas, 59, 28, mood);

    canvas_draw_str(canvas, 59, 41, level);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, 123 - exp, 48, exp + 1, 6);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_line(canvas, 123 - exp, 48, 123 - exp, 54);

    release_mutex((ValueMutex*)ctx, state);
}

int32_t passport(void* p) {
    DolphinState* dolphin_state = dolphin_state_alloc();
    ValueMutex state_mutex;
    dolphin_state_load(dolphin_state);

    osMessageQueueId_t event_queue = osMessageQueueNew(2, sizeof(AppEvent), NULL);
    furi_check(event_queue);

    if(!init_mutex(&state_mutex, dolphin_state, sizeof(DolphinState*))) {
        printf("[Passport] cannot create mutex\r\n");
        return 0;
    }

    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    AppEvent event;
    while(1) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 25);
        if(event_status == osOK) {
            if(event.type == EventTypeKey && event.value.input.type == InputTypeShort &&
               event.value.input.key == InputKeyBack) {
                break;
            }
        }

        view_port_update(view_port);
    }

    gui_remove_view_port(gui, view_port);

    view_port_free(view_port);

    furi_record_close("gui");

    delete_mutex(&state_mutex);

    osMessageQueueDelete(event_queue);

    dolphin_state_free(dolphin_state);

    return 0;
}
