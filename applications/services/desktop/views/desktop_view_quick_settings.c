#include <furi.h>
#include <math.h>
#include <gui/elements.h>
#include <assets_icons.h>

#include "../desktop_i.h"
#include "desktop_view_quick_settings.h"

typedef enum {
    DesktopQuickSettingsIndexBrightness,
    DesktopQuickSettingsIndexVolume,
    DesktopQuickSettingsIndexVibro,

    DesktopQuickSettingsIndexTotalCount
} DesktopQuickSettingsIndex;

static const char* const desktop_quick_settings_labels[DesktopQuickSettingsIndexTotalCount] = {
    "Brightness",
    "Volume",
    "Vibro",
};

// Rows keep the pitch and the frame of the first lock menu page, so paging sideways
// does not move anything but the contents of the rows.
#define QUICK_SETTINGS_ROW_HEIGHT    17
#define QUICK_SETTINGS_FRAME_X       15
#define QUICK_SETTINGS_FRAME_WIDTH   98
#define QUICK_SETTINGS_FRAME_HEIGHT  15
#define QUICK_SETTINGS_LABEL_X       19
#define QUICK_SETTINGS_CONTROL_RIGHT 104
#define QUICK_SETTINGS_CONTROL_MIN   34
#define QUICK_SETTINGS_CHECKBOX_SIZE 11

static uint8_t desktop_quick_settings_level_from_value(float value) {
    return CLAMP(
        (int32_t)roundf(value * (DESKTOP_QUICK_SETTINGS_LEVELS - 1)),
        DESKTOP_QUICK_SETTINGS_LEVELS - 1,
        0);
}

static float desktop_quick_settings_value_from_level(uint8_t level) {
    return (float)level / (DESKTOP_QUICK_SETTINGS_LEVELS - 1);
}

void desktop_quick_settings_set_callback(
    DesktopQuickSettingsView* quick_settings,
    DesktopQuickSettingsViewCallback callback,
    void* context) {
    furi_assert(quick_settings);
    furi_assert(callback);
    quick_settings->callback = callback;
    quick_settings->context = context;
}

void desktop_quick_settings_reset(
    DesktopQuickSettingsView* quick_settings,
    float brightness,
    float volume,
    bool vibro) {
    furi_assert(quick_settings);
    with_view_model(
        quick_settings->view,
        DesktopQuickSettingsViewModel * model,
        {
            model->idx = 0;
            model->editing = false;
            model->brightness = desktop_quick_settings_level_from_value(brightness);
            model->volume = desktop_quick_settings_level_from_value(volume);
            model->vibro = vibro;
        },
        true);
}

float desktop_quick_settings_get_brightness(DesktopQuickSettingsView* quick_settings) {
    furi_assert(quick_settings);
    uint8_t level = 0;
    with_view_model(
        quick_settings->view,
        DesktopQuickSettingsViewModel * model,
        { level = model->brightness; },
        false);
    return desktop_quick_settings_value_from_level(level);
}

float desktop_quick_settings_get_volume(DesktopQuickSettingsView* quick_settings) {
    furi_assert(quick_settings);
    uint8_t level = 0;
    with_view_model(
        quick_settings->view,
        DesktopQuickSettingsViewModel * model,
        { level = model->volume; },
        false);
    return desktop_quick_settings_value_from_level(level);
}

bool desktop_quick_settings_get_vibro(DesktopQuickSettingsView* quick_settings) {
    furi_assert(quick_settings);
    bool vibro = false;
    with_view_model(
        quick_settings->view,
        DesktopQuickSettingsViewModel * model,
        { vibro = model->vibro; },
        false);
    return vibro;
}

static void desktop_quick_settings_draw_callback(Canvas* canvas, void* model) {
    DesktopQuickSettingsViewModel* m = model;

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_icon(canvas, -57, 0 + STATUS_BAR_Y_SHIFT, &I_DoorLeft_70x55);
    canvas_draw_icon(canvas, 116, 0 + STATUS_BAR_Y_SHIFT, &I_DoorRight_70x55);
    canvas_set_font(canvas, FontSecondary);

    // All three controls start past the longest label so they line up in one column,
    // with the last few pixels of the row left free for the adjust arrows.
    size_t label_width = 0;
    for(size_t i = 0; i < DesktopQuickSettingsIndexTotalCount; ++i) {
        label_width = MAX(
            label_width, (size_t)canvas_string_width(canvas, desktop_quick_settings_labels[i]));
    }
    const int32_t control_x =
        MIN((int32_t)(QUICK_SETTINGS_LABEL_X + label_width + 8),
            (int32_t)(QUICK_SETTINGS_CONTROL_RIGHT - QUICK_SETTINGS_CONTROL_MIN));
    const size_t control_width = QUICK_SETTINGS_CONTROL_RIGHT - control_x;

    for(size_t i = 0; i < DesktopQuickSettingsIndexTotalCount; ++i) {
        const int32_t top = 1 + (i * QUICK_SETTINGS_ROW_HEIGHT) + STATUS_BAR_Y_SHIFT;

        canvas_set_color(canvas, ColorBlack);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(
            canvas,
            QUICK_SETTINGS_LABEL_X,
            top + 7,
            AlignLeft,
            AlignCenter,
            desktop_quick_settings_labels[i]);

        if(i == DesktopQuickSettingsIndexVibro) {
            const int32_t box_x = control_x + (control_width - QUICK_SETTINGS_CHECKBOX_SIZE) / 2;
            canvas_draw_rframe(
                canvas,
                box_x,
                top + 2,
                QUICK_SETTINGS_CHECKBOX_SIZE,
                QUICK_SETTINGS_CHECKBOX_SIZE,
                2);
            if(m->vibro) canvas_draw_box(canvas, box_x + 3, top + 5, 5, 5);
        } else {
            const uint8_t level = (i == DesktopQuickSettingsIndexBrightness) ? m->brightness :
                                                                               m->volume;
            char value[8];
            snprintf(value, sizeof(value), "%d%%", level * 5);
            elements_progress_bar_with_text(
                canvas,
                control_x,
                top + 2,
                control_width,
                desktop_quick_settings_value_from_level(level),
                value);
        }

        if(m->idx != i) continue;

        canvas_set_color(canvas, ColorBlack);
        elements_frame(
            canvas,
            QUICK_SETTINGS_FRAME_X,
            top,
            QUICK_SETTINGS_FRAME_WIDTH,
            QUICK_SETTINGS_FRAME_HEIGHT);

        if(m->editing) {
            canvas_draw_icon(canvas, control_x - 6, top + 5, &I_ButtonLeftSmall_3x5);
            canvas_draw_icon(
                canvas, QUICK_SETTINGS_CONTROL_RIGHT + 3, top + 5, &I_ButtonRightSmall_3x5);
        }
    }
}

View* desktop_quick_settings_get_view(DesktopQuickSettingsView* quick_settings) {
    furi_assert(quick_settings);
    return quick_settings->view;
}

static bool desktop_quick_settings_input_callback(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    DesktopQuickSettingsView* quick_settings = context;
    bool consumed = false;
    bool update = false;
    bool notify = false;
    DesktopEvent desktop_event = DesktopQuickSettingsEventSave;

    with_view_model(
        quick_settings->view,
        DesktopQuickSettingsViewModel * model,
        {
            // Holding a direction keeps stepping, so a slider can be dragged in one press.
            const bool step = (event->type == InputTypeShort) || (event->type == InputTypeRepeat);

            if(event->key == InputKeyBack) {
                // Back leaves the item being edited; only when nothing is being edited
                // does it fall through to the scene manager and take us back a page.
                if((event->type == InputTypeShort) && model->editing) {
                    model->editing = false;
                    update = true;
                    consumed = true;
                    notify = true;
                }
            } else if(event->key == InputKeyOk) {
                if(event->type == InputTypeShort) {
                    if(model->idx == DesktopQuickSettingsIndexVibro) {
                        // Nothing to slide on a checkbox, so it flips on the spot and
                        // never enters the edit mode the two bars use.
                        model->vibro = !model->vibro;
                        desktop_event = DesktopQuickSettingsEventVibroChanged;
                        notify = true;
                    } else {
                        model->editing = !model->editing;
                        notify = !model->editing;
                    }
                    update = true;
                    consumed = true;
                }
            } else if(((event->key == InputKeyUp) || (event->key == InputKeyDown)) && step) {
                if(!model->editing) {
                    if(event->key == InputKeyUp) {
                        model->idx = (model->idx == 0) ? DesktopQuickSettingsIndexTotalCount - 1 :
                                                         model->idx - 1;
                    } else {
                        model->idx = (model->idx == DesktopQuickSettingsIndexTotalCount - 1) ?
                                         0 :
                                         model->idx + 1;
                    }
                    update = true;
                }
                consumed = true;
            } else if(((event->key == InputKeyLeft) || (event->key == InputKeyRight)) && step) {
                const bool forward = (event->key == InputKeyRight);
                if(model->editing) {
                    // Only the two bars can be in edit mode, so this is one of them.
                    const bool brightness = (model->idx == DesktopQuickSettingsIndexBrightness);
                    uint8_t* level = brightness ? &model->brightness : &model->volume;
                    const uint8_t next = CLAMP(
                        (int32_t)*level + (forward ? 1 : -1),
                        DESKTOP_QUICK_SETTINGS_LEVELS - 1,
                        0);
                    if(next != *level) {
                        *level = next;
                        update = true;
                        notify = true;
                        desktop_event = brightness ? DesktopQuickSettingsEventBrightnessChanged :
                                                     DesktopQuickSettingsEventVolumeChanged;
                    }
                } else if(event->type == InputTypeShort) {
                    // Sideways off the settings page goes back to the first menu page.
                    notify = true;
                    desktop_event = DesktopQuickSettingsEventClose;
                }
                consumed = true;
            }
        },
        update);

    // Outside the model lock: the callback ends up back in this view.
    if(notify) quick_settings->callback(desktop_event, quick_settings->context);

    return consumed;
}

DesktopQuickSettingsView* desktop_quick_settings_alloc(void) {
    DesktopQuickSettingsView* quick_settings = malloc(sizeof(DesktopQuickSettingsView));
    quick_settings->view = view_alloc();
    view_allocate_model(
        quick_settings->view, ViewModelTypeLocking, sizeof(DesktopQuickSettingsViewModel));
    view_set_context(quick_settings->view, quick_settings);
    view_set_draw_callback(quick_settings->view, desktop_quick_settings_draw_callback);
    view_set_input_callback(quick_settings->view, desktop_quick_settings_input_callback);

    return quick_settings;
}

void desktop_quick_settings_free(DesktopQuickSettingsView* quick_settings) {
    furi_assert(quick_settings);

    view_free(quick_settings->view);
    free(quick_settings);
}
