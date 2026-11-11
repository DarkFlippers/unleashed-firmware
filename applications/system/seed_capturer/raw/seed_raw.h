#pragma once

#include <furi.h>
#include <lib/subghz/receiver.h>

typedef struct SeedRaw SeedRaw;

SeedRaw* seed_raw_alloc(void);
void seed_raw_free(SeedRaw* instance);

/** Open a RAW .sub and feed its samples to the receiver instead of the radio. */
bool seed_raw_start(SeedRaw* instance, const char* path, SubGhzReceiver* receiver);

/** Decode one tick worth of samples. Returns false once the file is used up. */
bool seed_raw_next(SeedRaw* instance);

void seed_raw_stop(SeedRaw* instance);

/** "3/7 42%" style text from the worker, empty when not running. */
void seed_raw_progress(SeedRaw* instance, FuriString* out);
