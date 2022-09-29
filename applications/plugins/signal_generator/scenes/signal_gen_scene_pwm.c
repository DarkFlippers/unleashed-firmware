#include "../signal_gen_app_i.h"

static const FuriHalPwmOutputId pwm_ch_id[] = {
    FuriHalPwmOutputIdTim1PA7,
    FuriHalPwmOutputIdLptim2PA4,
};

#define DEFAULT_FREQ 1000
#define DEFAULT_DUTY 50

static void
    signal_gen_pwm_callback(uint8_t channel_id, uint32_t freq, uint8_t duty, void* context) {
    SignalGenApp* app = context;

    app->pwm_freq = freq;
    app->pwm_duty = duty;

    if(app->pwm_ch != pwm_ch_id[channel_id]) {
        app->pwm_ch_prev = app->pwm_ch;
        app->pwm_ch = pwm_ch_id[channel_id];
        view_dispatcher_send_custom_event(app->view_dispatcher, SignalGenPwmEventChannelChange);
    } else {
        app->pwm_ch = pwm_ch_id[channel_id];
        view_dispatcher_send_custom_event(app->view_dispatcher, SignalGenPwmEventUpdate);
    }
}

void signal_gen_scene_pwm_on_enter(void* context) {
    SignalGenApp* app = context;

    view_dispatcher_switch_to_view(app->view_dispatcher, SignalGenViewPwm);

    signal_gen_pwm_set_callback(app->pwm_view, signal_gen_pwm_callback, app);

    signal_gen_pwm_set_params(app->pwm_view, 0, DEFAULT_FREQ, DEFAULT_DUTY);
    furi_hal_pwm_start(pwm_ch_id[0], DEFAULT_FREQ, DEFAULT_DUTY);
}

bool signal_gen_scene_pwm_on_event(void* context, SceneManagerEvent event) {
    SignalGenApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SignalGenPwmEventUpdate) {
            consumed = true;
            furi_hal_pwm_set_params(app->pwm_ch, app->pwm_freq, app->pwm_duty);
        } else if(event.event == SignalGenPwmEventChannelChange) {
            consumed = true;
            furi_hal_pwm_stop(app->pwm_ch_prev);
            furi_hal_pwm_start(app->pwm_ch, app->pwm_freq, app->pwm_duty);
        }
    }
    return consumed;
}

void signal_gen_scene_pwm_on_exit(void* context) {
    SignalGenApp* app = context;
    variable_item_list_reset(app->var_item_list);
    furi_hal_pwm_stop(app->pwm_ch);
}
