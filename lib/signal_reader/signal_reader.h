#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <furi_hal_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SignalReaderEventTypeHalfBufferFilled,
    SignalReaderEventTypeFullBufferFilled,
} SignalReaderEventType;

typedef struct {
    uint8_t* data;
    size_t len;
} SignalReaderEventData;

typedef struct {
    SignalReaderEventType type;
    SignalReaderEventData* data;
} SignalReaderEvent;

typedef enum {
    SignalReaderTimeUnit64Mhz,
} SignalReaderTimeUnit;

typedef enum {
    SignalReaderPolarityNormal,
    SignalReaderPolarityInverted,
} SignalReaderPolarity;

typedef enum {
    SignalReaderTriggerNone,
    SignalReaderTriggerRisingFallingEdge,
} SignalReaderTrigger;

typedef void (*SignalReaderCallback)(SignalReaderEvent event, void* context);

typedef struct SignalReader SignalReader;

SignalReader* signal_reader_alloc(const GpioPin* gpio_pin, uint32_t size);

void signal_reader_free(SignalReader* instance);

void signal_reader_set_pull(SignalReader* instance, GpioPull pull);

void signal_reader_set_polarity(SignalReader* instance, SignalReaderPolarity polarity);

void signal_reader_set_sample_rate(
    SignalReader* instance,
    SignalReaderTimeUnit time_unit,
    uint32_t time);

void signal_reader_set_trigger(SignalReader* instance, SignalReaderTrigger trigger);

void signal_reader_start(SignalReader* instance, SignalReaderCallback callback, void* context);

void signal_reader_stop(SignalReader* instance);

#ifdef __cplusplus
}
#endif
