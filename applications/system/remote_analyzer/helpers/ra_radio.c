#include "ra_radio.h"
#include "ra_worker.h"
#include "../protocols/ra_protocol_items.h"

#include <furi.h>
#include <furi_hal.h>

#define TAG "RaRadio"

struct RaRadio {
    SubGhzSetting* setting;
    SubGhzEnvironment* environment;
    const SubGhzProtocolRegistry* registry;
    SubGhzReceiver* receiver;
    RaWorker* worker;

    SubGhzRadioPreset preset;

    bool database_loaded;
    bool speaker_wanted;
    bool speaker_mine;
    bool rx_running;
};

static void ra_radio_pair_callback(void* context, bool level, uint32_t duration) {
    RaRadio* instance = context;
    subghz_receiver_decode(instance->receiver, level, duration);
}

static void ra_radio_overrun_callback(void* context) {
    RaRadio* instance = context;
    subghz_receiver_reset(instance->receiver);
}

RaRadio* ra_radio_alloc(void) {
    RaRadio* instance = malloc(sizeof(RaRadio));

    instance->setting = subghz_setting_alloc();
    // Picks up the user's own frequencies and custom presets, same file the Sub-GHz app reads
    subghz_setting_load(instance->setting, EXT_PATH("subghz/assets/setting_user"));

    instance->environment = subghz_environment_alloc();
    instance->database_loaded =
        subghz_environment_load_keystore(instance->environment, SUBGHZ_KEYSTORE_DIR_NAME);
    subghz_environment_load_keystore(instance->environment, SUBGHZ_KEYSTORE_DIR_USER_NAME);
    subghz_environment_set_alutech_at_4n_rainbow_table_file_name(
        instance->environment, SUBGHZ_ALUTECH_AT_4N_DIR_NAME);
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(
        instance->environment, SUBGHZ_NICE_FLOR_S_DIR_NAME);

    instance->registry = ra_protocol_registry_alloc();
    subghz_environment_set_protocol_registry(instance->environment, instance->registry);
    // Allocates one decoder per protocol and nothing else - no encoder is ever touched
    instance->receiver = subghz_receiver_alloc_init(instance->environment);
    // Decodable only - BinRAW is deliberately not part of this app
    subghz_receiver_set_filter(instance->receiver, SubGhzProtocolFlag_Decodable);

    instance->worker =
        ra_worker_alloc(ra_radio_pair_callback, ra_radio_overrun_callback, instance);

    instance->preset.name = furi_string_alloc();
    instance->preset.frequency = 0;
    instance->preset.data = NULL;
    instance->preset.data_size = 0;

    instance->speaker_wanted = false;
    instance->speaker_mine = false;
    instance->rx_running = false;

    return instance;
}

void ra_radio_free(RaRadio* instance) {
    furi_assert(instance);

    ra_radio_stop(instance);
    ra_radio_sleep(instance);

    ra_worker_free(instance->worker);
    subghz_receiver_free(instance->receiver);
    subghz_environment_free(instance->environment);
    ra_protocol_registry_free(instance->registry);
    furi_string_free(instance->preset.name);
    subghz_setting_free(instance->setting);

    free(instance);
}

SubGhzSetting* ra_radio_get_setting(RaRadio* instance) {
    furi_assert(instance);
    return instance->setting;
}

SubGhzReceiver* ra_radio_get_receiver(RaRadio* instance) {
    furi_assert(instance);
    return instance->receiver;
}

bool ra_radio_is_database_loaded(RaRadio* instance) {
    furi_assert(instance);
    return instance->database_loaded;
}

void ra_radio_set_rx_callback(RaRadio* instance, SubGhzReceiverCallback callback, void* context) {
    furi_assert(instance);
    subghz_receiver_set_rx_callback(instance->receiver, callback, context);
}

void ra_radio_set_speaker(RaRadio* instance, bool enable) {
    furi_assert(instance);
    instance->speaker_wanted = enable;
}

void ra_radio_set_preset(RaRadio* instance, uint32_t frequency, size_t preset_index) {
    furi_assert(instance);

    const char* name = subghz_setting_get_preset_name(instance->setting, preset_index);
    furi_string_set(instance->preset.name, name ? name : "");
    instance->preset.frequency = frequency;
    instance->preset.data = subghz_setting_get_preset_data(instance->setting, preset_index);
    instance->preset.data_size =
        subghz_setting_get_preset_data_size(instance->setting, preset_index);
}

SubGhzRadioPreset ra_radio_get_preset(RaRadio* instance) {
    furi_assert(instance);
    return instance->preset;
}

void ra_radio_get_frequency_and_modulation(
    RaRadio* instance,
    FuriString* frequency,
    FuriString* modulation) {
    furi_assert(instance);

    if(frequency != NULL) {
        furi_string_printf(
            frequency,
            "%03ld.%02ld",
            instance->preset.frequency / 1000000 % 1000,
            instance->preset.frequency / 10000 % 100);
    }
    if(modulation != NULL) {
        furi_string_set(modulation, instance->preset.name);
    }
}

static void ra_radio_speaker_on(RaRadio* instance) {
    if(!instance->speaker_wanted) return;
    if(furi_hal_speaker_acquire(30)) {
        instance->speaker_mine = true;
        furi_hal_subghz_set_async_mirror_pin(&gpio_speaker);
    }
}

static void ra_radio_speaker_off(RaRadio* instance) {
    if(!instance->speaker_mine) return;
    furi_hal_subghz_set_async_mirror_pin(NULL);
    if(furi_hal_speaker_is_mine()) furi_hal_speaker_release();
    instance->speaker_mine = false;
}

void ra_radio_rx_start(RaRadio* instance) {
    furi_assert(instance);

    if(instance->preset.data == NULL) {
        FURI_LOG_E(TAG, "No preset to start with");
        return;
    }

    ra_radio_stop(instance);

    furi_hal_subghz_reset();
    furi_hal_subghz_idle();
    furi_hal_subghz_load_custom_preset(instance->preset.data);
    furi_hal_subghz_idle();
    furi_hal_subghz_set_frequency_and_path(instance->preset.frequency);
    furi_hal_subghz_flush_rx();

    // The mirror pin has to be in place before the capture starts driving it
    ra_radio_speaker_on(instance);

    furi_hal_subghz_start_async_rx(ra_worker_rx_callback, instance->worker);
    ra_worker_start(instance->worker);
    instance->rx_running = true;
}

void ra_radio_stop(RaRadio* instance) {
    furi_assert(instance);
    if(!instance->rx_running) return;

    // Join the worker first: it is what feeds the decoders, and it must be idle before
    // anything else is torn down
    ra_worker_stop(instance->worker);
    furi_hal_subghz_stop_async_rx();
    furi_hal_subghz_idle();
    ra_radio_speaker_off(instance);
    instance->rx_running = false;
}

void ra_radio_sleep(RaRadio* instance) {
    furi_assert(instance);
    furi_check(!instance->rx_running);
    furi_hal_subghz_sleep();
}

void ra_radio_reset_decoders(RaRadio* instance) {
    furi_assert(instance);
    subghz_receiver_reset(instance->receiver);
    subghz_environment_reset_keeloq(instance->environment);
}
