/**
 * @file furi_hal_pwm.h
 * PWM contol HAL
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    FuriHalPwmOutputIdNone,
    FuriHalPwmOutputIdTim1PA7,
    FuriHalPwmOutputIdLptim2PA4,
} FuriHalPwmOutputId;

/** Enable PWM channel and set parameters
 * 
 * @param[in]  channel  PWM channel (FuriHalPwmOutputId)
 * @param[in]  freq  Frequency in Hz
 * @param[in]  duty  Duty cycle value in %
*/
void furi_hal_pwm_start(FuriHalPwmOutputId channel, uint32_t freq, uint8_t duty);

/** Disable PWM channel
 * 
 * @param[in]  channel  PWM channel (FuriHalPwmOutputId)
*/
void furi_hal_pwm_stop(FuriHalPwmOutputId channel);

/** Set PWM channel parameters
 * 
 * @param[in]  channel  PWM channel (FuriHalPwmOutputId)
 * @param[in]  freq  Frequency in Hz
 * @param[in]  duty  Duty cycle value in %
*/
void furi_hal_pwm_set_params(FuriHalPwmOutputId channel, uint32_t freq, uint8_t duty);

/** Is PWM channel running?
 * 
 * @param[in]  channel  PWM channel (FuriHalPwmOutputId)
 * @return bool - true if running
*/
bool furi_hal_pwm_is_running(FuriHalPwmOutputId channel);

#ifdef __cplusplus
}
#endif
