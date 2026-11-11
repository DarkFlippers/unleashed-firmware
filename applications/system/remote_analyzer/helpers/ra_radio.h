/**
 * @file ra_radio.h
 * @brief Receive-only backend on the internal CC1101.
 *
 * Nothing here touches the Sub-GHz device registry: that loads external-module plugins off
 * the SD card and keeps them resident, and this app is internal-radio only. It talks to
 * furi_hal_subghz directly instead.
 *
 * What it allocates: the frequency/preset table from the Sub-GHz config, the keystore the
 * dynamic decoders need, the combined protocol registry's pointer table, one decoder per
 * protocol, and the capture worker. No encoders, no transmitter, no history.
 */
#pragma once

#include <lib/subghz/receiver.h>
#include <lib/subghz/subghz_setting.h>
#include <lib/subghz/types.h>

typedef struct RaRadio RaRadio;

RaRadio* ra_radio_alloc(void);

void ra_radio_free(RaRadio* instance);

/** The frequency and preset table, loaded from subghz/assets/setting_user. App-owned. */
SubGhzSetting* ra_radio_get_setting(RaRadio* instance);

/** The receiver the decoders live in, so a file's samples can be fed to the same set. */
SubGhzReceiver* ra_radio_get_receiver(RaRadio* instance);

/** False when the KeeLoq manufacturer keystore is missing, so names will read "Unknown". */
bool ra_radio_is_database_loaded(RaRadio* instance);

/** Called from the worker thread when a protocol decodes a frame. */
void ra_radio_set_rx_callback(RaRadio* instance, SubGhzReceiverCallback callback, void* context);

/** Mirror the demodulator onto the speaker while receiving. Takes effect on the next start. */
void ra_radio_set_speaker(RaRadio* instance, bool enable);

/** Tune, by frequency and an index into the setting's preset list. */
void ra_radio_set_preset(RaRadio* instance, uint32_t frequency, size_t preset_index);

SubGhzRadioPreset ra_radio_get_preset(RaRadio* instance);

/** "433.92" and "AM650" for the header. Either output may be NULL. */
void ra_radio_get_frequency_and_modulation(
    RaRadio* instance,
    FuriString* frequency,
    FuriString* modulation);

void ra_radio_rx_start(RaRadio* instance);

void ra_radio_stop(RaRadio* instance);

void ra_radio_sleep(RaRadio* instance);

/** Drop every decoder's partial frame, and the sticky KeeLoq manufacturer guess with it. */
void ra_radio_reset_decoders(RaRadio* instance);
