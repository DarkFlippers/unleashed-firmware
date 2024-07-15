#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <furi_hal_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PULSE_READER_NO_EDGE   (0xFFFFFFFFUL)
#define PULSE_READER_LOST_EDGE (0xFFFFFFFEUL)
#define F_TIM2                 (64000000UL)

/**
 * unit of the edge durations to return
 */
typedef enum {
    PulseReaderUnit64MHz,
    PulseReaderUnitPicosecond,
    PulseReaderUnitNanosecond,
    PulseReaderUnitMicrosecond,
} PulseReaderUnit;

/* using an anonymous type */
typedef struct PulseReader PulseReader;

/** Allocate a PulseReader object
 *
 * Allocates memory for a ringbuffer and initalizes the object
 *
 * @param[in]  gpio        the GPIO to use. will get configured as input.
 * @param[in]  size        number of edges to buffer
 */
PulseReader* pulse_reader_alloc(const GpioPin* gpio, uint32_t size);

/** Free a PulseReader object
 *
 * Frees all memory of the given object
 *
 * @param[in]  signal      previously allocated PulseReader object.
 */
void pulse_reader_free(PulseReader* signal);

/** Start signal capturing
 *
 * Initializes DMA1, TIM2 and DMAMUX_REQ_GEN_0 to automatically capture timer values.
 * Ensure that interrupts are always enabled, as the used EXTI line is handled as one.
 *
 * @param[in]  signal      previously allocated PulseReader object.
 */
void pulse_reader_start(PulseReader* signal);

/** Stop signal capturing
 *
 * Frees DMA1, TIM2 and DMAMUX_REQ_GEN_0
 *
 * @param[in]  signal      previously allocated PulseReader object.
 */
void pulse_reader_stop(PulseReader* signal);

/** Recevie a sample from ringbuffer
 *
 * Waits for the specified time until a new edge gets detected.
 * If not configured otherwise, the pulse duration will be in picosecond resolution.
 * If a bittime was configured, the return value will contain the properly rounded
 * number of bit times measured.
 *
 * @param[in]  signal      previously allocated PulseReader object.
 * @param[in]  timeout_us  time to wait for a signal [µs]
 *
 * @returns the scaled value of the pulse duration
 */
uint32_t pulse_reader_receive(PulseReader* signal, int timeout_us);

/** Get available samples
 *
 * Get the number of available samples in the ringbuffer
 *
 * @param[in]  signal  previously allocated PulseReader object.
 *
 * @returns the number of samples in buffer
 */
uint32_t pulse_reader_samples(PulseReader* signal);

/** Set timebase
 *
 * Set the timebase to be used when returning pulse duration.
 *
 * @param[in]  signal  previously allocated PulseReader object.
 * @param[in]  unit  PulseReaderUnit64MHz or PulseReaderUnitPicosecond
 */
void pulse_reader_set_timebase(PulseReader* signal, PulseReaderUnit unit);

/** Set bit time
 *
 * Set the number of timebase units per bit.
 * When set, the pulse_reader_receive() will return an already rounded
 * bit count value instead of the raw duration.
 *
 * Set to 1 to return duration again.
 *
 * @param[in]  signal    previously allocated PulseReader object.
 * @param[in]  bit_time
 */
void pulse_reader_set_bittime(PulseReader* signal, uint32_t bit_time);

/** Set GPIO pull direction
 *
 * Some GPIOs need pulldown, others don't. By default the
 * pull direction is GpioPullNo.
 *
 * @param[in]  signal    previously allocated PulseReader object.
 * @param[in]  pull      GPIO pull direction
 */
void pulse_reader_set_pull(PulseReader* signal, GpioPull pull);

#ifdef __cplusplus
}
#endif
