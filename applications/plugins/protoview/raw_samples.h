/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

/* Our circular buffer of raw samples, used in order to display
 * the signal. */

#define RAW_SAMPLES_NUM \
    2048 /* Use a power of two: we take the modulo
                                of the index quite often to normalize inside
                                the range, and division is slow. */
typedef struct RawSamplesBuffer {
    FuriMutex* mutex;
    struct {
        uint16_t level : 1;
        uint16_t dur : 15;
    } samples[RAW_SAMPLES_NUM];
    uint32_t idx; /* Current idx (next to write). */
    uint32_t total; /* Total samples: same as RAW_SAMPLES_NUM, we provide
                       this field for a cleaner interface with the user, but
                       we always use RAW_SAMPLES_NUM when taking the modulo so
                       the compiler can optimize % as bit masking. */
    /* Signal features. */
    uint32_t short_pulse_dur; /* Duration of the shortest pulse. */
} RawSamplesBuffer;

RawSamplesBuffer* raw_samples_alloc(void);
void raw_samples_reset(RawSamplesBuffer* s);
void raw_samples_center(RawSamplesBuffer* s, uint32_t offset);
void raw_samples_add(RawSamplesBuffer* s, bool level, uint32_t dur);
void raw_samples_add_or_update(RawSamplesBuffer* s, bool level, uint32_t dur);
void raw_samples_get(RawSamplesBuffer* s, uint32_t idx, bool* level, uint32_t* dur);
void raw_samples_copy(RawSamplesBuffer* dst, RawSamplesBuffer* src);
void raw_samples_free(RawSamplesBuffer* s);
