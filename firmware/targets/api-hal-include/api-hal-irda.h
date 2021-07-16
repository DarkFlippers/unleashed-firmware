#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Signature of callback function for receiving continuous IRDA rx signal.
 *
 * @param   ctx[in] - context to pass to callback
 * @param   level[in] - level of input IRDA rx signal
 * @param   duration[in] - duration of continuous rx signal level in us
 */
typedef void (*ApiHalIrdaCaptureCallback)(void* ctx, bool level, uint32_t duration);

/**
 * Signature of callback function for reaching silence timeout on IRDA port.
 *
 * @param   ctx[in] - context to pass to callback
 */
typedef void (*ApiHalIrdaTimeoutCallback)(void* ctx);

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

/** Setup api hal for receiving silence timeout.
 * Should be used with 'api_hal_irda_timeout_irq_set_callback()'.
 *
 * @param[in]   timeout_ms - time to wait for silence on IRDA port
 *                           before generating IRQ.
 */
void api_hal_irda_rx_timeout_irq_init(uint32_t timeout_ms);

/**
 * Setup callback for previously initialized IRDA RX interrupt.
 *
 * @param[in]   callback - callback to call when RX signal edge changing occurs
 * @param[in]   ctx - context for callback
 */
void api_hal_irda_rx_irq_set_callback(ApiHalIrdaCaptureCallback callback, void *ctx);

/**
 * Setup callback for reaching silence timeout on IRDA port.
 * Should setup api hal with 'api_hal_irda_setup_rx_timeout_irq()' first.
 *
 * @param[in]   callback - callback for silence timeout
 * @param[in]   ctx - context to pass to callback
 */
void api_hal_irda_rx_timeout_irq_set_callback(ApiHalIrdaTimeoutCallback callback, void *ctx);

/**
 * Start generating IRDA TX PWM. Provides PWM initialization on
 * defined frequency.
 *
 * @param[in]   duty_cycle - duty cycle
 * @param[in]   freq - PWM frequency to generate
 */
void api_hal_irda_pwm_set(float duty_cycle, float freq);

/**
 * Stop generating IRDA PWM signal.
 */
void api_hal_irda_pwm_stop();

/**
 * Check if IRDA is in use now.
 * @return  false - IRDA is busy, true otherwise.
 */
bool api_hal_irda_rx_irq_is_busy(void);

#ifdef __cplusplus
}
#endif

