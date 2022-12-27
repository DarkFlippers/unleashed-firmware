#include "countdown_view.h"
#include "../utils/utils.h"

// internal
static void handle_misc_cmd(CountDownTimView* hw, CountDownViewCmd cmd);
static void handle_time_setting_updown(CountDownTimView* cdv, CountDownViewCmd cmd);
static void handle_time_setting_select(InputKey key, CountDownTimView* cdv);
static void draw_selection(Canvas* canvas, CountDownViewSelect selection);

static void countdown_timer_start_counting(CountDownTimView* cdv);
static void countdown_timer_pause_counting(CountDownTimView* cdv);

// callbacks
static void countdown_timer_view_on_enter(void* ctx);
static void countdown_timer_view_on_draw(Canvas* canvas, void* ctx);
static bool countdown_timer_view_on_input(InputEvent* event, void* ctx);
static void timer_cb(void* ctx);

CountDownTimView* countdown_timer_view_new() {
    CountDownTimView* cdv = (CountDownTimView*)(malloc(sizeof(CountDownTimView)));

    cdv->view = view_alloc();

    cdv->timer = furi_timer_alloc(timer_cb, FuriTimerTypePeriodic, cdv);

    cdv->counting = false;

    view_set_context(cdv->view, cdv);

    view_allocate_model(cdv->view, ViewModelTypeLocking, sizeof(CountDownModel));

    view_set_draw_callback(cdv->view, countdown_timer_view_on_draw);
    view_set_input_callback(cdv->view, countdown_timer_view_on_input);
    view_set_enter_callback(cdv->view, countdown_timer_view_on_enter);

    return cdv;
}

void countdown_timer_view_delete(CountDownTimView* cdv) {
    furi_assert(cdv);

    view_free(cdv->view);
    furi_timer_stop(cdv->timer);
    furi_timer_free(cdv->timer);

    free(cdv);
}

View* countdown_timer_view_get_view(CountDownTimView* cdv) {
    return cdv->view;
}

void countdown_timer_view_state_reset(CountDownTimView* cdv) {
    cdv->counting = false;

    with_view_model(
        cdv->view, CountDownModel * model, { model->count = model->saved_count_setting; }, true)
}

void countdown_timer_state_toggle(CountDownTimView* cdv) {
    bool on = cdv->counting;
    if(!on) {
        countdown_timer_start_counting(cdv);
    } else {
        countdown_timer_pause_counting(cdv);
    }

    cdv->counting = !on;
}

// on enter callback, CountDownTimView as ctx
static void countdown_timer_view_on_enter(void* ctx) {
    furi_assert(ctx);

    CountDownTimView* cdv = (CountDownTimView*)ctx;

    // set current count to a initial value
    with_view_model(
        cdv->view,
        CountDownModel * model,
        {
            model->count = INIT_COUNT;
            model->saved_count_setting = INIT_COUNT;
        },
        true);
}

// view draw callback, CountDownModel as ctx
static void countdown_timer_view_on_draw(Canvas* canvas, void* ctx) {
    furi_assert(ctx);
    CountDownModel* model = (CountDownModel*)ctx;

    char buffer[64];

    int32_t count = model->count;
    int32_t expected_count = model->saved_count_setting;

    CountDownViewSelect select = model->select;

    // elements_frame(canvas, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    canvas_set_font(canvas, FontBigNumbers);
    draw_selection(canvas, select);

    parse_sec_to_time_str(buffer, sizeof(buffer), count);
    canvas_draw_str_aligned(
        canvas, SCREEN_CENTER_X, SCREEN_CENTER_Y, AlignCenter, AlignCenter, buffer);

    elements_progress_bar(canvas, 0, 0, SCREEN_WIDTH, (1.0 * count / expected_count));
}

// keys input event callback, CountDownTimView as ctx
static bool countdown_timer_view_on_input(InputEvent* event, void* ctx) {
    furi_assert(ctx);

    CountDownTimView* hw = (CountDownTimView*)ctx;

    if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
        switch(event->key) {
        case InputKeyUp:
        case InputKeyDown:
        case InputKeyRight:
        case InputKeyLeft:
            handle_time_setting_select(event->key, hw);
            break;

        case InputKeyOk:
            if(event->type == InputTypeShort) {
                handle_misc_cmd(hw, CountDownTimerToggleCounting);
            }
            break;

        default:
            break;
        }

        return true;
    }

    if(event->type == InputTypeLong) {
        switch(event->key) {
        case InputKeyOk:
            handle_misc_cmd(hw, CountDownTimerReset);
            break;

        case InputKeyBack:
            return false;
            break;

        default:
            break;
        }

        return true;
    }

    return false;
}

static void timer_cb(void* ctx) {
    furi_assert(ctx);

    CountDownTimView* cdv = (CountDownTimView*)ctx;

    int32_t count;
    bool timeup = false;

    // decrement counter
    with_view_model(
        cdv->view,
        CountDownModel * model,
        {
            count = model->count;
            count--;

            // check timeup
            if(count <= 0) {
                count = 0;
                timeup = true;
            }

            model->count = count;
        },
        true);

    if(timeup) {
        handle_misc_cmd(cdv, CountDownTimerTimeUp);
    }
}

static void handle_time_setting_updown(CountDownTimView* cdv, CountDownViewCmd cmd) {
    int32_t count;

    with_view_model(
        cdv->view,
        CountDownModel * model,
        {
            count = model->count;
            switch(cmd) {
            case CountDownTimerMinuteUp:
                count += 60;
                break;
            case CountDownTimerMinuteDown:
                count -= 60;
                break;
            case CountDownTimerHourDown:
                count -= 3600;
                break;
            case CountDownTimerHourUp:
                count += 3600;
                break;
            case CountDownTimerSecUp:
                count++;
                break;
            case CountDownTimerSecDown:
                count--;
                break;
            default:
                break;
            }

            if(count < 0) {
                count = 0;
            }

            // update count state
            model->count = count;

            // save the count time setting
            model->saved_count_setting = count;
        },
        true);
}

static void handle_misc_cmd(CountDownTimView* hw, CountDownViewCmd cmd) {
    switch(cmd) {
    case CountDownTimerTimeUp:
        notification_timeup();
        break;

    case CountDownTimerReset:
        furi_timer_stop(hw->timer);
        countdown_timer_view_state_reset(hw);
        notification_off();

        break;

    case CountDownTimerToggleCounting:
        countdown_timer_state_toggle(hw);
        break;

    default:
        break;
    }

    return;
}

static void handle_time_setting_select(InputKey key, CountDownTimView* cdv) {
    bool counting = cdv->counting;
    CountDownViewCmd setting_cmd = CountDownTimerSecUp;
    CountDownViewSelect selection;

    if(counting) {
        return;
    }

    // load current selection from model context
    with_view_model(
        cdv->view, CountDownModel * model, { selection = model->select; }, false);

    // select
    switch(key) {
    case InputKeyUp:
        switch(selection) {
        case CountDownTimerSelectSec:
            setting_cmd = CountDownTimerSecUp;
            break;
        case CountDownTimerSelectMinute:
            setting_cmd = CountDownTimerMinuteUp;
            break;
        case CountDownTimerSelectHour:
            setting_cmd = CountDownTimerHourUp;
            break;
        }

        handle_time_setting_updown(cdv, setting_cmd);
        break;

    case InputKeyDown:
        switch(selection) {
        case CountDownTimerSelectSec:
            setting_cmd = CountDownTimerSecDown;
            break;
        case CountDownTimerSelectMinute:
            setting_cmd = CountDownTimerMinuteDown;
            break;
        case CountDownTimerSelectHour:
            setting_cmd = CountDownTimerHourDown;
            break;
        }

        handle_time_setting_updown(cdv, setting_cmd);
        break;

    case InputKeyRight:
        selection--;
        selection = selection % 3;
        break;

    case InputKeyLeft:
        selection++;
        selection = selection % 3;
        break;

    default:
        break;
    }

    // save selection to model context
    with_view_model(
        cdv->view, CountDownModel * model, { model->select = selection; }, false);
}

static void draw_selection(Canvas* canvas, CountDownViewSelect selection) {
    switch(selection) {
    case CountDownTimerSelectSec:
        elements_slightly_rounded_box(canvas, SCREEN_CENTER_X + 25, SCREEN_CENTER_Y + 11, 24, 2);
        break;
    case CountDownTimerSelectMinute:
        elements_slightly_rounded_box(canvas, SCREEN_CENTER_X - 10, SCREEN_CENTER_Y + 11, 21, 2);
        break;
    case CountDownTimerSelectHour:
        elements_slightly_rounded_box(canvas, SCREEN_CENTER_X - 47, SCREEN_CENTER_Y + 11, 24, 2);
        break;
    }
}

static void countdown_timer_start_counting(CountDownTimView* cdv) {
    furi_timer_start(cdv->timer, furi_kernel_get_tick_frequency() * 1); // 1s
}

static void countdown_timer_pause_counting(CountDownTimView* cdv) {
    furi_timer_stop(cdv->timer);
    notification_off();
}