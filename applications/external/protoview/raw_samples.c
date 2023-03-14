/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#include <inttypes.h>
#include <furi/core/string.h>
#include <furi.h>
#include <furi_hal.h>
#include "raw_samples.h"

/* Allocate and initialize a samples buffer. */
RawSamplesBuffer* raw_samples_alloc(void) {
    RawSamplesBuffer* buf = malloc(sizeof(*buf));
    buf->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    raw_samples_reset(buf);
    return buf;
}

/* Free a sample buffer. Should be called when the mutex is released. */
void raw_samples_free(RawSamplesBuffer* s) {
    furi_mutex_free(s->mutex);
    free(s);
}

/* This just set all the samples to zero and also resets the internal
 * index. There is no need to call it after raw_samples_alloc(), but only
 * when one wants to reset the whole buffer of samples. */
void raw_samples_reset(RawSamplesBuffer* s) {
    furi_mutex_acquire(s->mutex, FuriWaitForever);
    s->total = RAW_SAMPLES_NUM;
    s->idx = 0;
    s->short_pulse_dur = 0;
    memset(s->samples, 0, sizeof(s->samples));
    furi_mutex_release(s->mutex);
}

/* Set the raw sample internal index so that what is currently at
 * offset 'offset', will appear to be at 0 index. */
void raw_samples_center(RawSamplesBuffer* s, uint32_t offset) {
    s->idx = (s->idx + offset) % RAW_SAMPLES_NUM;
}

/* Add the specified sample in the circular buffer. */
void raw_samples_add(RawSamplesBuffer* s, bool level, uint32_t dur) {
    furi_mutex_acquire(s->mutex, FuriWaitForever);
    s->samples[s->idx].level = level;
    s->samples[s->idx].dur = dur;
    s->idx = (s->idx + 1) % RAW_SAMPLES_NUM;
    furi_mutex_release(s->mutex);
}

/* This is like raw_samples_add(), however in case a sample of the
 * same level of the previous one is added, the duration of the last
 * sample is updated instead. Needed mainly for the decoders build_message()
 * methods: it is simpler to write an encoder of a signal like that,
 * just creating messages piece by piece.
 *
 * This function is a bit slower so the internal data sampling should
 * be performed with raw_samples_add(). */
void raw_samples_add_or_update(RawSamplesBuffer* s, bool level, uint32_t dur) {
    furi_mutex_acquire(s->mutex, FuriWaitForever);
    uint32_t previdx = (s->idx - 1) % RAW_SAMPLES_NUM;
    if(s->samples[previdx].level == level && s->samples[previdx].dur != 0) {
        /* Update the last sample: it has the same level. */
        s->samples[previdx].dur += dur;
    } else {
        /* Add a new sample. */
        s->samples[s->idx].level = level;
        s->samples[s->idx].dur = dur;
        s->idx = (s->idx + 1) % RAW_SAMPLES_NUM;
    }
    furi_mutex_release(s->mutex);
}

/* Get the sample from the buffer. It is possible to use out of range indexes
 * as 'idx' because the modulo operation will rewind back from the start. */
void raw_samples_get(RawSamplesBuffer* s, uint32_t idx, bool* level, uint32_t* dur) {
    furi_mutex_acquire(s->mutex, FuriWaitForever);
    idx = (s->idx + idx) % RAW_SAMPLES_NUM;
    *level = s->samples[idx].level;
    *dur = s->samples[idx].dur;
    furi_mutex_release(s->mutex);
}

/* Copy one buffer to the other, including current index. */
void raw_samples_copy(RawSamplesBuffer* dst, RawSamplesBuffer* src) {
    furi_mutex_acquire(src->mutex, FuriWaitForever);
    furi_mutex_acquire(dst->mutex, FuriWaitForever);
    dst->idx = src->idx;
    dst->short_pulse_dur = src->short_pulse_dur;
    memcpy(dst->samples, src->samples, sizeof(dst->samples));
    furi_mutex_release(src->mutex);
    furi_mutex_release(dst->mutex);
}
