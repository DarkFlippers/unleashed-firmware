/**
  * @file infrared_parser.h
  * Infrared: Helper file for conversion Flipper File Format
  *     to Infrared signal class, and backwards
  */
#pragma once

#include "../infrared_app_signal.h"
#include <flipper_format/flipper_format.h>
#include <string>

/** Save Infrared signal into file
 *
 * @param ff - Flipper File Format instance
 * @param signal - Infrared signal to save
 * @param name - name for saved signal. Every
 *      signal on disk has name.
 */
bool infrared_parser_save_signal(
    FlipperFormat* ff,
    const InfraredAppSignal& signal,
    const std::string& name);

/** Read Infrared signal from file
 *
 * @param ff - Flipper File Format instance
 * @param signal - Infrared signal to read to
 * @param name - name for saved signal. Every
 *      signal in file has name.
 */
bool infrared_parser_read_signal(FlipperFormat* ff, InfraredAppSignal& signal, std::string& name);

/** Validate parsed signal
 *
 * @signal - signal to validate
 * @retval true if valid, false otherwise
 */
bool infrared_parser_is_parsed_signal_valid(const InfraredMessage* signal);

/** Validate raw signal
 *
 * @signal - signal to validate
 * @retval true if valid, false otherwise
 */
bool infrared_parser_is_raw_signal_valid(
    uint32_t frequency,
    float duty_cycle,
    uint32_t timings_cnt);
