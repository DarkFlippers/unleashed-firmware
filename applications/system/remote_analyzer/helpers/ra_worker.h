/**
 * @file ra_worker.h
 * @brief Capture worker: carries edges from the CC1101 capture ISR to the decoders.
 *
 * Same shape as the firmware's SubGhzWorker, with a quarter of the queue. The firmware sizes
 * its queue for 4096 edges (16KB); a remote frame is a few hundred, and the consumer drains
 * one edge in microseconds, so the rest of that buffer only ever absorbs scheduling jitter.
 * 1024 edges is 4KB, and those 12KB matter more here than the margin does.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct RaWorker RaWorker;

typedef void (*RaWorkerPairCallback)(void* context, bool level, uint32_t duration);
typedef void (*RaWorkerOverrunCallback)(void* context);

/** Hand to furi_hal_subghz_start_async_rx() with the worker as context. Runs in the ISR. */
void ra_worker_rx_callback(bool level, uint32_t duration, void* context);

RaWorker* ra_worker_alloc(
    RaWorkerPairCallback pair_callback,
    RaWorkerOverrunCallback overrun_callback,
    void* context);

void ra_worker_free(RaWorker* instance);

void ra_worker_start(RaWorker* instance);

/** Joins the worker thread, so no callback is in flight once this returns. */
void ra_worker_stop(RaWorker* instance);

bool ra_worker_is_running(RaWorker* instance);
