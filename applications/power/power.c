#include "power.h"

#include <flipper_v2.h>
#include <gui/gui.h>
#include <gui/widget.h>
#include <assets_icons.h>

#define BATTERY_MIN_VOLTAGE 3.2f
#define BATTERY_MAX_VOLTAGE 4.0f
#define BATTERY_INIT 0xFFAACCEE

extern ADC_HandleTypeDef hadc1;

struct Power {
    Icon* usb_icon;
    Widget* usb_widget;

    Icon* battery_icon;
    Widget* battery_widget;

    uint32_t charge;
};

void power_draw_usb_callback(CanvasApi* canvas, void* context) {
    assert(context);
    Power* power = context;
    canvas->draw_icon(canvas, 0, 0, power->usb_icon);
}

void power_draw_battery_callback(CanvasApi* canvas, void* context) {
    assert(context);
    Power* power = context;

    canvas->draw_icon(canvas, 0, 0, power->battery_icon);

    if(power->charge != BATTERY_INIT) {
        float charge = ((float)power->charge / 1000 * 2 - BATTERY_MIN_VOLTAGE) /
                       (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE);
        if(charge > 1) {
            charge = 1;
        }
        canvas->draw_box(canvas, 2, 2, charge * 14, 4);
    }
}

void power_input_events_callback(const void* value, void* ctx) {
    assert(ctx);
    Power* power = ctx;
    InputEvent* event = value;

    if(event->input != InputCharging) return;

    widget_enabled_set(power->usb_widget, event->state);
    widget_update(power->usb_widget);
}

Power* power_alloc() {
    Power* power = furi_alloc(sizeof(Power));

    power->usb_icon = assets_icons_get(I_USBConnected_15x8);
    power->usb_widget = widget_alloc();
    widget_set_width(power->usb_widget, icon_get_width(power->usb_icon));

    ValueManager* input_state_manager = furi_open("input_state");
    InputState input_state;
    read_mutex_block(input_state_manager, &input_state, sizeof(input_state));
    widget_enabled_set(power->usb_widget, input_state.charging);

    widget_draw_callback_set(power->usb_widget, power_draw_usb_callback, power);

    power->battery_icon = assets_icons_get(I_Battery_19x8);
    power->battery_widget = widget_alloc();
    widget_set_width(power->battery_widget, icon_get_width(power->battery_icon));
    widget_draw_callback_set(power->battery_widget, power_draw_battery_callback, power);

    PubSub* input_event_record = furi_open("input_events");
    assert(input_event_record);
    subscribe_pubsub(input_event_record, power_input_events_callback, power);

    power->charge = BATTERY_INIT;

    return power;
}

void power_free(Power* power) {
    assert(power);
    free(power);
}

void power_task(void* p) {
    (void)p;
    Power* power = power_alloc();

    FuriRecordSubscriber* gui_record = furi_open_deprecated("gui", false, false, NULL, NULL, NULL);
    assert(gui_record);
    GuiApi* gui = furi_take(gui_record);
    assert(gui);
    gui->add_widget(gui, power->usb_widget, GuiLayerStatusBarLeft);
    gui->add_widget(gui, power->battery_widget, GuiLayerStatusBarRight);
    furi_commit(gui_record);

    if(!furi_create("power", power)) {
        printf("[power_task] unable to create power record\n");
        furiac_exit(NULL);
    }

    furiac_ready();

    while(1) {
        HAL_ADC_Start(&hadc1);
        if(HAL_ADC_PollForConversion(&hadc1, 1000) != HAL_TIMEOUT) {
            power->charge = HAL_ADC_GetValue(&hadc1);
            widget_update(power->battery_widget);
        }
        osDelay(1000);
    }
}
