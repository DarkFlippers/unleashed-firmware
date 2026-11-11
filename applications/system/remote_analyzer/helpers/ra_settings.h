/**
 * @file ra_settings.h
 * @brief What the app remembers between runs, in apps_data/remote_analyzer/settings.
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define RA_SETTINGS_DEFAULT_FREQUENCY 433920000
/** Index into the preset list loaded from the Sub-GHz config: 1 is AM650. */
#define RA_SETTINGS_DEFAULT_PRESET    1
#define RA_SETTINGS_DEFAULT_TRIGGER   (-93.0f)
/** Read screen: dBm above which something is transmitting rather than the band being noisy. */
#define RA_SETTINGS_DEFAULT_RSSI      (-85.0f)

typedef struct {
    uint32_t frequency;
    uint32_t preset_index; // AKA Modulation
    bool sound;
    bool sweep; // try the other modulations when a signal will not decode
    bool hopping; // walk the hopper frequency list while the air is quiet
    float rssi_trigger; // what counts as a signal, for the sweep, the hop and the LED
    bool alert; // vibro and a chirp on a code that is not a repeat of the last one
    float fa_trigger; // frequency analyzer RSSI trigger
    uint32_t fa_feedback; // frequency analyzer feedback level
} RaSettings;

/** Fill in defaults, then whatever the file has. @p preset_count clamps a stale preset index. */
void ra_settings_load(RaSettings* instance, size_t preset_count);

bool ra_settings_save(const RaSettings* instance);
