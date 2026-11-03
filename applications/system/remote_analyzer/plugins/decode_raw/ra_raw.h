/**
 * @file ra_raw.h
 * @brief Feed a RAW .sub capture to the decoders in place of the radio.
 *
 * The samples go straight into the receiver, so a file is decoded against exactly the same
 * protocol set as a live signal, with no radio claimed and no module needed.
 */
#pragma once

#include <furi.h>
#include <lib/subghz/receiver.h>

typedef struct RaRaw RaRaw;

RaRaw* ra_raw_alloc(void);
void ra_raw_free(RaRaw* instance);
/** Open @p path and point its samples at @p receiver. False when the file will not open. */
bool ra_raw_start(RaRaw* instance, const char* path, SubGhzReceiver* receiver);
/** Decode one batch of samples. False once the file is used up. */
bool ra_raw_next(RaRaw* instance);
void ra_raw_stop(RaRaw* instance);
bool ra_raw_is_running(const RaRaw* instance);
/** How far through the file, 0 to 100. */
uint8_t ra_raw_percent(RaRaw* instance);

/**
 * What @p path says about itself before any sample is read.
 *
 * @param[out] frequency what it was recorded on, or 0 when the file does not say. May be NULL.
 * @return false unless it is a RAW capture, the only kind that holds samples at all
 */
bool ra_raw_file_is_raw(const char* path, uint32_t* frequency);
