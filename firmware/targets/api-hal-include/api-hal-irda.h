#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Signature of callback function for receiving continuous IRDA rx signal.
 *
 * @param   level - level of input IRDA rx signal
 * @param   duration - duration of continuous rx signal level in us
 */
typedef void (*TimerISRCallback)(void* ctx, bool level, uint32_t duration);


/**
 * Initialize IRDA RX timer to receive interrupts.
 * It provides interrupts for every RX-signal edge changing
 * with its duration.
 */
void api_hal_irda_rx_irq_init(void);

/**
 * Deinitialize IRDA RX interrupt.
 */
void api_hal_irda_rx_irq_deinit(void);

/**
 * Setup callback for previously initialized IRDA RX interrupt.
 *
 * @param   callback - callback to call when RX signal edge changing occurs
 * @param   ctx - context for callback
 */
void api_hal_irda_rx_irq_set_callback(TimerISRCallback callback, void *ctx);

/**
 * Start generating IRDA TX PWM. Provides PWM initialization on
 * defined frequency.
 *
 * @param   duty_cycle - duty cycle
 * @param   freq - PWM frequency to generate
 */
void api_hal_irda_pwm_set(float duty_cycle, float freq);

/**
 * Stop generating IRDA PWM signal.
 */
void api_hal_irda_pwm_stop();

#ifdef __cplusplus
}
#endif

