#include <furi.h>
#include <math.h>
#include <stddef.h>
#include <gui/elements.h>
#include <assets_icons.h>

#include "../desktop_i.h"
#include "desktop_view_quick_settings.h"

typedef enum {
    DesktopQuickSettingsIndexBrightness,
    DesktopQuickSettingsIndexVolume,
    DesktopQuickSettingsIndexBluetooth,
    DesktopQuickSettingsIndexDummy,
    DesktopQuickSettingsIndexVibro,
    DesktopQuickSettingsIndexMute,
    DesktopQuickSettingsIndexHand,
    DesktopQuickSettingsIndexTotalCount
} DesktopQuickSettingsIndex;

// One page: two direct-adjust vertical sliders, a 2x2 toggle grid and hand button.
#define HYBRIDUS_SLIDER_TOP 18
#define HYBRIDUS_SLIDER_HEIGHT 43
#define HYBRIDUS_SLIDER_WIDTH 17

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
    bool vibro,
    bool dummy) {
    furi_assert(quick_settings);
    with_view_model(
        quick_settings->view,
        DesktopQuickSettingsViewModel * model,
        {
            model->idx = 0;
            model->last_button = DesktopQuickSettingsIndexBluetooth;
            model->button_column = 0;
            model->restore_volume = 10;
            model->bluetooth = false;
            model->left_hand = false;
            model->brightness = desktop_quick_settings_level_from_value(brightness);
            model->volume = desktop_quick_settings_level_from_value(volume);
            model->vibro = vibro;
            model->dummy = dummy;
        },
        true);
}

// All seven getters take the same model lock and read exactly one byte. Keep
// that lock/read/unlock sequence in one place instead of storing seven copies
// in MCU flash. offsetof binds callers to the actual private model layout;
// unsigned char may alias any object, including bool, under the C aliasing rules.
static __attribute__((noinline)) uint8_t desktop_quick_settings_get_field(
    DesktopQuickSettingsView* quick_settings, size_t offset) {
    furi_assert(quick_settings);
    uint8_t value = 0;
    with_view_model(quick_settings->view, DesktopQuickSettingsViewModel * model,
                   {value = ((const unsigned char*)model)[offset];}, false);
    return value;
}

float desktop_quick_settings_get_brightness(DesktopQuickSettingsView* quick_settings) {
    return desktop_quick_settings_value_from_level(desktop_quick_settings_get_field(quick_settings, offsetof(DesktopQuickSettingsViewModel, brightness)));
}

float desktop_quick_settings_get_volume(DesktopQuickSettingsView* quick_settings) {
    return desktop_quick_settings_value_from_level(desktop_quick_settings_get_field(quick_settings, offsetof(DesktopQuickSettingsViewModel, volume)));
}

bool desktop_quick_settings_get_vibro(DesktopQuickSettingsView* quick_settings) {
    return desktop_quick_settings_get_field(quick_settings, offsetof(DesktopQuickSettingsViewModel, vibro));
}

bool desktop_quick_settings_get_dummy(DesktopQuickSettingsView* quick_settings) {
    return desktop_quick_settings_get_field(quick_settings, offsetof(DesktopQuickSettingsViewModel, dummy));
}

void desktop_quick_settings_set_system_state(
    DesktopQuickSettingsView* quick_settings, bool bluetooth, bool left_hand, uint8_t restore_volume) {
    with_view_model(quick_settings->view, DesktopQuickSettingsViewModel * model, {
        model->bluetooth = bluetooth;
        model->left_hand = left_hand;
        model->restore_volume = CLAMP(restore_volume, 20, 1);
    }, true);
}

bool desktop_quick_settings_get_bluetooth(DesktopQuickSettingsView* quick_settings) {
    return desktop_quick_settings_get_field(quick_settings, offsetof(DesktopQuickSettingsViewModel, bluetooth));
}
bool desktop_quick_settings_get_left_hand(DesktopQuickSettingsView* quick_settings) {
    return desktop_quick_settings_get_field(quick_settings, offsetof(DesktopQuickSettingsViewModel, left_hand));
}
uint8_t desktop_quick_settings_get_restore_volume(DesktopQuickSettingsView* quick_settings) {
    return desktop_quick_settings_get_field(quick_settings, offsetof(DesktopQuickSettingsViewModel, restore_volume));
}

static void desktop_quick_settings_button(
    Canvas* canvas, int x, int y, int w, const char* label, bool on, bool selected) {
    if(on) {
        canvas_draw_rbox(canvas, x, y, w, 14, 3);
        canvas_set_color(canvas, ColorWhite);
    } else {
        canvas_draw_rframe(canvas, x, y, w, 14, 3);
    }
    canvas_draw_str_aligned(canvas, x + w / 2, y + 7, AlignCenter, AlignCenter, label);
    canvas_set_color(canvas, ColorBlack);
    if(selected) elements_frame(canvas, x - 1, y - 1, w + 2, 15);
}

static void desktop_quick_settings_draw_callback(Canvas* canvas, void* model) {
    DesktopQuickSettingsViewModel* m = model;
    canvas_set_color(canvas, ColorBlack);
    // Reuse the original Unleashed door art at its original clipped positions.
    // The controls now live strictly between the door edges (x=14..113).
    canvas_draw_icon(canvas, -57, STATUS_BAR_Y_SHIFT, &I_DoorLeft_70x55);
    canvas_draw_icon(canvas, 116, STATUS_BAR_Y_SHIFT, &I_DoorRight_70x55);
    canvas_set_font(canvas, FontSecondary);
    for(uint8_t i = 0; i < 2; ++i) {
        const int x = 15 + 20 * i;
        const int y = HYBRIDUS_SLIDER_TOP;
        const int w = HYBRIDUS_SLIDER_WIDTH;
        const int h = HYBRIDUS_SLIDER_HEIGHT;
        const uint8_t level = i == 0 ? m->brightness : m->volume;
        const int fill = ((h - 2) * level) / 20;
        const Icon* icon = i == 0 ? &I_Pin_star_7x7 : &I_Volup_8x6;
        if(m->idx == i) elements_bold_rounded_frame(canvas, x - 1, y - 1, w + 1, h + 2);
        canvas_draw_rframe(canvas, x, y, w, h, 4);
        canvas_draw_icon(canvas, x + (w - icon_get_width(icon)) / 2, y + 6, icon);
        char text[5];
        snprintf(text, sizeof(text), "%u", (unsigned)level * 5);
        canvas_draw_str_aligned(canvas, x + w / 2, y + 30, AlignCenter, AlignCenter, text);
        if(fill) {
            canvas_set_color(canvas, ColorXOR);
            canvas_draw_box(canvas, x + 1, y + h - 1 - fill, w - 2, fill);
        }
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_rframe(canvas, x, y, w, h, 4);
    }
    desktop_quick_settings_button(canvas, 54, 17, 20, "BT", m->bluetooth, m->idx == 2);
    desktop_quick_settings_button(canvas, 77, 17, 36, "Dummy", m->dummy, m->idx == 3);
    desktop_quick_settings_button(canvas, 54, 33, 26, "Vibro", m->vibro, m->idx == 4);
    desktop_quick_settings_button(canvas, 83, 33, 28, "Mute", m->volume == 0, m->idx == 5);
    desktop_quick_settings_button(canvas, 54, 49, 57,
                                 m->left_hand ? "Left hand" : "Right hand",
                                 m->left_hand, m->idx == 6);
}

View* desktop_quick_settings_get_view(DesktopQuickSettingsView* quick_settings) {
    furi_assert(quick_settings);
    return quick_settings->view;
}

static bool desktop_quick_settings_input_callback(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    DesktopQuickSettingsView* quick_settings = context;
    if(event->key == InputKeyBack) {
        if(event->type == InputTypeLong) {
            // Keep access to the original Unleashed toggles, including Stealth.
            quick_settings->callback(DesktopQuickSettingsEventClose, quick_settings->context);
            return true;
        }
        return false;
    }
    if(event->type != InputTypeShort && event->type != InputTypeRepeat) return true;
    bool notify = false;
    DesktopEvent desktop_event = DesktopQuickSettingsEventSave;
    with_view_model(quick_settings->view, DesktopQuickSettingsViewModel * model, {
        const uint8_t old_idx = model->idx;
        if(event->key == InputKeyLeft || event->key == InputKeyRight) {
            // Horizontal movement always moves focus; no hidden edit/page mode.
            if(event->key == InputKeyLeft) {
                if(model->idx == 1) model->idx = 0;
                else if(model->idx == 3 || model->idx == 5) model->idx--;
                else if(model->idx >= 2) model->idx = 1;
            } else {
                if(model->idx == 0) model->idx = 1;
                else if(model->idx == 1) model->idx = model->last_button;
                else if(model->idx == 2 || model->idx == 4) model->idx++;
            }
            if(model->idx >= 2) {
                model->last_button = model->idx;
                if(model->idx < 6) model->button_column = model->idx % 2;
            }
            notify = model->idx != old_idx;
        } else if(event->key == InputKeyUp || event->key == InputKeyDown) {
            if(model->idx <= 1) {
                uint8_t* level = model->idx == 0 ? &model->brightness : &model->volume;
                const uint8_t next = CLAMP((int)*level + (event->key == InputKeyUp ? 1 : -1), 20, 0);
                if(*level != next) {
                    *level = next;
                    if(model->idx == 1 && next) model->restore_volume = next;
                    desktop_event = model->idx == 0 ? DesktopQuickSettingsEventBrightnessChanged :
                                                     DesktopQuickSettingsEventVolumeChanged;
                    notify = true;
                }
            } else {
                if(event->key == InputKeyDown) {
                    if(model->idx < 4) model->idx += 2;
                    else if(model->idx < 6) model->idx = 6;
                } else {
                    if(model->idx == 6) model->idx = 4 + model->button_column;
                    else if(model->idx >= 4) model->idx -= 2;
                }
                model->last_button = model->idx;
                if(model->idx < 6) model->button_column = model->idx % 2;
            }
        } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
            switch(model->idx) {
            case DesktopQuickSettingsIndexBluetooth:
                model->bluetooth = !model->bluetooth;
                desktop_event = DesktopQuickSettingsEventBluetoothChanged;
                notify = true;
                break;
            case DesktopQuickSettingsIndexDummy:
                model->dummy = !model->dummy;
                desktop_event = DesktopQuickSettingsEventDummyChanged;
                notify = true;
                break;
            case DesktopQuickSettingsIndexVibro:
                model->vibro = !model->vibro;
                desktop_event = DesktopQuickSettingsEventVibroChanged;
                notify = true;
                break;
            case DesktopQuickSettingsIndexMute:
                if(model->volume) {
                    model->restore_volume = model->volume;
                    model->volume = 0;
                } else {
                    model->volume = model->restore_volume;
                }
                desktop_event = DesktopQuickSettingsEventMuteChanged;
                notify = true;
                break;
            case DesktopQuickSettingsIndexHand:
                model->left_hand = !model->left_hand;
                desktop_event = DesktopQuickSettingsEventHandChanged;
                notify = true;
                break;
            default:
                break;
            }
        }
    }, true);
    if(notify) quick_settings->callback(desktop_event, quick_settings->context);
    return true;
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
