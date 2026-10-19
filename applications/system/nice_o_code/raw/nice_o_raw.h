#pragma once

#include <furi.h>
#include <lib/subghz/receiver.h>

typedef struct NiceORaw NiceORaw;

NiceORaw* nice_o_raw_alloc(void);
void nice_o_raw_free(NiceORaw* instance);

/** Open a RAW .sub and feed its samples to the receiver instead of the radio. */
bool nice_o_raw_start(NiceORaw* instance, const char* path, SubGhzReceiver* receiver);

/** Decode one tick worth of samples. Returns false once the file is used up. */
bool nice_o_raw_next(NiceORaw* instance);

void nice_o_raw_stop(NiceORaw* instance);

/** "3/7 42%" style text from the worker, empty when not running. */
void nice_o_raw_progress(NiceORaw* instance, FuriString* out);
