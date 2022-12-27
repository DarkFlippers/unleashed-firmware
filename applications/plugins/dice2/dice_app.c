#include <furi.h>
#include <input/input.h>
#include <gui/gui.h>
#include "constants.h"

const Icon* draw_dice_frame;

static void update(State* const state) {
    if(state->app_state == SwipeLeftState) {
        for(uint8_t i = 0; i < DICE_TYPES; i++) {
            state->dices[i].x -= SWIPE_DIST;
            state->dices[i].y = DICE_Y;
        }

        if(state->dices[state->dice_index].x == DICE_X) {
            state->app_state = SelectState;
            state->dices[state->dice_index].y = DICE_Y_T;
        }

    } else if(state->app_state == SwipeRightState) {
        for(uint8_t i = 0; i < DICE_TYPES; i++) {
            state->dices[i].x += SWIPE_DIST;
            state->dices[i].y = DICE_Y;
        }

        if(state->dices[state->dice_index].x == DICE_X) {
            state->app_state = SelectState;
            state->dices[state->dice_index].y = DICE_Y_T;
        }
    } else if(state->app_state == AnimState) {
        state->anim_frame += 1;

        if(state->dice_index == 0) {
            if(state->anim_frame == 3) coin_set_start(state->roll_result); // change coin anim

            if(state->anim_frame >= MAX_COIN_FRAMES) {
                state->anim_frame = 0;
                state->app_state = AnimResultState;
            }
        } else {
            if(state->anim_frame >= MAX_DICE_FRAMES) {
                state->anim_frame = 0;
                state->app_state = AnimResultState;
            }
        }
    } else if(state->app_state == AnimResultState) {
        if(state->dice_index == 0) { // no extra animations for coin
            state->anim_frame = 0;
            state->app_state = ResultState;
            return;
        }

        state->result_pos = result_frame_pos_y[state->anim_frame];
        state->anim_frame += 1;

        // end animation
        if(state->result_pos == 0) {
            state->anim_frame = 0;
            state->app_state = ResultState;
        }
    }
}

static void roll(State* const state) {
    state->roll_result = 0;
    state->result_pos = result_frame_pos_y[0];

    for(uint8_t i = 0; i < MAX_DICE_COUNT; i++) {
        if(i < state->dice_count) {
            state->rolled_dices[i] = (rand() % dice_types[state->dice_index].type) + 1;
            state->roll_result += state->rolled_dices[i];
        } else {
            state->rolled_dices[i] = 0;
        }
    }

    if(state->dice_index == 0) coin_set_end(state->roll_result); // change coin anim

    state->app_state = AnimState;
}

static void draw_ui(const State* state, Canvas* canvas) {
    canvas_set_font(canvas, FontSecondary);

    FuriString* count = furi_string_alloc();
    furi_string_printf(count, "%01d", state->dice_count);

    // dice name
    if(isDiceNameVisible(state->app_state)) {
        canvas_draw_str_aligned(
            canvas, 63, 50, AlignCenter, AlignBottom, dice_types[state->dice_index].name);
    }
    // dice arrow buttons
    if(isDiceButtonsVisible(state->app_state)) {
        if(state->dice_index > 0) canvas_draw_icon(canvas, 45, 44, &I_ui_button_left);
        if(state->dice_index < DICE_TYPES - 1)
            canvas_draw_icon(canvas, 78, 44, &I_ui_button_right);
    }

    // dice count settings
    if(isDiceSettingsDisabled(state->app_state, state->dice_index))
        canvas_draw_icon(canvas, 48, 51, &I_ui_count_1);
    else
        canvas_draw_icon(canvas, 48, 51, &I_ui_count);
    canvas_draw_str_aligned(canvas, 58, 61, AlignCenter, AlignBottom, furi_string_get_cstr(count));

    // buttons
    if(isAnimState(state->app_state) == false) canvas_draw_icon(canvas, 92, 54, &I_ui_button_roll);

    if(state->app_state != AnimResultState && state->app_state != ResultState) {
        canvas_draw_icon(canvas, 0, 54, &I_ui_button_exit);
    } else {
        canvas_draw_icon(canvas, 0, 54, &I_ui_button_back);
    }

    furi_string_free(count);
}

static void draw_dice(const State* state, Canvas* canvas) {
    if(isMenuState(state->app_state) == false) { // draw only selected dice
        if(state->dice_index == 0) { // coin
            draw_dice_frame = coin_frames[state->anim_frame];
        } else { // dices
            draw_dice_frame =
                dice_frames[(state->dice_index - 1) * MAX_DICE_FRAMES + state->anim_frame];
        }

        canvas_draw_icon(
            canvas,
            state->dices[state->dice_index].x,
            state->dices[state->dice_index].y,
            draw_dice_frame);
        return;
    }

    for(uint8_t i = 0; i < DICE_TYPES; i++) {
        if(state->app_state == ResultState && state->dice_index == i && state->dice_index != 0)
            continue; // draw results except coin
        if(state->dices[i].x > 128 || state->dices[i].x < -35) continue; // outside the screen

        if(i == 0) { // coin
            draw_dice_frame = coin_frames[0];
        } else { // dices
            draw_dice_frame = dice_frames[(i - 1) * MAX_DICE_FRAMES];
        }

        canvas_draw_icon(canvas, state->dices[i].x, state->dices[i].y, draw_dice_frame);
    }
}

static void draw_results(const State* state, Canvas* canvas) {
    canvas_set_font(canvas, FontPrimary);

    FuriString* sum = furi_string_alloc();
    furi_string_printf(sum, "%01d", state->roll_result);

    // ui frame
    if(state->app_state == AnimResultState)
        canvas_draw_icon(canvas, RESULT_BORDER_X, state->result_pos, &I_ui_result_border);
    else
        canvas_draw_icon(
            canvas, RESULT_BORDER_X, result_frame_pos_y[MAX_DICE_FRAMES - 1], &I_ui_result_border);

    // result text
    canvas_draw_str_aligned(
        canvas,
        64,
        state->result_pos + RESULT_OFFSET,
        AlignCenter,
        AlignCenter,
        furi_string_get_cstr(sum));

    if(state->app_state == ResultState && isOneDice(state->dice_index) == false) {
        canvas_set_font(canvas, FontSecondary);

        FuriString* dices = furi_string_alloc();
        for(uint8_t i = 0; i < state->dice_count; i++) {
            furi_string_cat_printf(dices, "%01d", state->rolled_dices[i]);

            if(i != state->dice_count - 1) furi_string_cat_printf(dices, "%s", ", ");
        }

        canvas_draw_str_aligned(
            canvas, 63, 37, AlignCenter, AlignCenter, furi_string_get_cstr(dices));
        furi_string_free(dices);
    }

    furi_string_free(sum);
}

static void draw_callback(Canvas* canvas, void* ctx) {
    const State* state = acquire_mutex((ValueMutex*)ctx, 25);
    if(state == NULL) {
        return;
    }

    canvas_clear(canvas);

    draw_ui(state, canvas);

    if(isResultVisible(state->app_state, state->dice_index)) {
        draw_results(state, canvas);
    } else {
        draw_dice(state, canvas);
    }

    release_mutex((ValueMutex*)ctx, state);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    AppEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    AppEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

int32_t dice_dnd_app(void* p) {
    UNUSED(p);

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(AppEvent));

    FURI_LOG_E(TAG, ">>> Started...\r\n");
    State* state = malloc(sizeof(State));
    init(state);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, state, sizeof(State))) {
        FURI_LOG_E(TAG, "cannot create mutex\r\n");
        free(state);
        return 255;
    }

    // Set callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    FuriTimer* timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() * 0.2);

    // Create GUI, register view port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    AppEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        State* state = (State*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            // timer evetn
            if(event.type == EventTypeTick) {
                update(state);
            }
            // button events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    // dice type
                    if(isDiceButtonsVisible(state->app_state)) {
                        if(event.input.key == InputKeyRight) {
                            if(state->dice_index < DICE_TYPES - 1) {
                                state->dice_index += 1;
                                state->app_state = SwipeLeftState;
                            }
                        } else if(event.input.key == InputKeyLeft) {
                            if(state->dice_index > 0) {
                                state->dice_index -= 1;
                                state->app_state = SwipeRightState;
                            }
                        }

                        if(isOneDice(state->dice_index)) state->dice_count = 1;
                    }
                    // dice count
                    if(isDiceSettingsDisabled(state->app_state, state->dice_index) == false &&
                       isAnimState(state->app_state) == false) {
                        if(event.input.key == InputKeyUp) {
                            if(state->dice_index != 0) {
                                state->dice_count += 1;
                                if(state->dice_count > MAX_DICE_COUNT) {
                                    state->dice_count = MAX_DICE_COUNT;
                                }
                            }
                        } else if(event.input.key == InputKeyDown) {
                            state->dice_count -= 1;
                            if(state->dice_count < 1) {
                                state->dice_count = 1;
                            }
                        }
                    }
                    // roll
                    if(event.input.key == InputKeyOk && isAnimState(state->app_state) == false) {
                        roll(state);
                    }
                    // back to dice select state or quit from app
                    if(event.input.key == InputKeyBack) {
                        if(state->app_state == ResultState ||
                           state->app_state == AnimResultState) {
                            state->anim_frame = 0;
                            state->app_state = SelectState;
                        } else {
                            processing = false;
                        }
                    }
                }
            }
        } else {
            FURI_LOG_D(TAG, "osMessageQueue: event timeout");
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, state);
    }

    // Clear
    free(state);
    furi_timer_free(timer);
    furi_message_queue_free(event_queue);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    delete_mutex(&state_mutex);

    return 0;
}