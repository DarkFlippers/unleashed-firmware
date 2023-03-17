#pragma once

#include <gui/view.h>
#include "../signal_gen_app_i.h"

typedef struct SignalGenPwm SignalGenPwm;
typedef void (
    *SignalGenPwmViewCallback)(uint8_t channel_id, uint32_t freq, uint8_t duty, void* context);

SignalGenPwm* signal_gen_pwm_alloc();

void signal_gen_pwm_free(SignalGenPwm* pwm);

View* signal_gen_pwm_get_view(SignalGenPwm* pwm);

void signal_gen_pwm_set_callback(
    SignalGenPwm* pwm,
    SignalGenPwmViewCallback callback,
    void* context);

void signal_gen_pwm_set_params(SignalGenPwm* pwm, uint8_t channel_id, uint32_t freq, uint8_t duty);
