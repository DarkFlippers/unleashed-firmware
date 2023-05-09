#pragma once
#include "subghz_types.h"
#include <lib/subghz/subghz_worker.h>
#include <lib/subghz/subghz_setting.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/protocols/raw.h>

typedef void (*SubGhzTxRxNeedSaveCallback)(void* context);

typedef struct SubGhzTxRx SubGhzTxRx;

typedef enum {
    SubGhzTxRxStartTxStateOk,
    SubGhzTxRxStartTxStateErrorOnlyRx,
    SubGhzTxRxStartTxStateErrorParserOthers,
} SubGhzTxRxStartTxState;

SubGhzTxRx* subghz_txrx_alloc();
void subghz_txrx_free(SubGhzTxRx* instance);
bool subghz_txrx_is_load_database(SubGhzTxRx* instance);

void subghz_txrx_set_preset(
    SubGhzTxRx* instance,
    const char* preset_name,
    uint32_t frequency,
    uint8_t* preset_data,
    size_t preset_data_size);

const char* subghz_txrx_get_name_preset(SubGhzTxRx* instance, const char* preset);
SubGhzRadioPreset subghz_txrx_get_preset(SubGhzTxRx* instance);

void subghz_txrx_get_frequency_modulation(
    SubGhzTxRx* instance,
    FuriString* frequency,
    FuriString* modulation,
    bool long_name);
SubGhzTxRxStartTxState subghz_txrx_tx_start(SubGhzTxRx* instance, FlipperFormat* flipper_format);
void subghz_txrx_rx_start(SubGhzTxRx* instance);
void subghz_txrx_stop(SubGhzTxRx* instance);
void subghz_txrx_sleep(SubGhzTxRx* instance);

void subghz_txrx_hopper_update(SubGhzTxRx* instance);
SubGhzHopperState subghz_txrx_hopper_get_state(SubGhzTxRx* instance);
void subghz_txrx_hopper_set_state(SubGhzTxRx* instance, SubGhzHopperState state);
void subghz_txrx_hopper_remove_pause(SubGhzTxRx* instance);
void subghz_txrx_hopper_pause(SubGhzTxRx* instance);

void subghz_txrx_speaker_on(SubGhzTxRx* instance);
void subghz_txrx_speaker_off(SubGhzTxRx* instance);
void subghz_txrx_speaker_mute(SubGhzTxRx* instance);
void subghz_txrx_speaker_unmute(SubGhzTxRx* instance);
void subghz_txrx_speaker_set_state(SubGhzTxRx* instance, SubGhzSpeakerState state);
SubGhzSpeakerState subghz_txrx_speaker_get_state(SubGhzTxRx* instance);
bool subghz_txrx_load_decoder_by_name_protocol(SubGhzTxRx* instance, const char* name_protocol);
SubGhzProtocolDecoderBase* subghz_txrx_get_decoder(SubGhzTxRx* instance);

void subghz_txrx_need_save_callback_set(
    SubGhzTxRx* instance,
    SubGhzTxRxNeedSaveCallback callback,
    void* context);
FlipperFormat* subghz_txtx_get_fff_data(SubGhzTxRx* instance);
SubGhzSetting* subghz_txrx_get_setting(SubGhzTxRx* instance);

bool subghz_txrx_protocol_is_preserved(SubGhzTxRx* instance);
bool subghz_txrx_protocol_is_send(SubGhzTxRx* instance, bool check_type);

void subghz_txrx_receiver_set_filter(SubGhzTxRx* instance, SubGhzProtocolFlag filter);

void subghz_txrx_set_rx_calback(
    SubGhzTxRx* instance,
    SubGhzReceiverCallback callback,
    void* context);
void subghz_txrx_set_raw_file_encoder_worker_set_callback_end(
    SubGhzTxRx* instance,
    SubGhzProtocolEncoderRAWCallbackEnd callback,
    void* context);

void subghz_txrx_set_debug_pin_state(SubGhzTxRx* instance, bool state);
bool subghz_txrx_get_debug_pin_state(SubGhzTxRx* instance);

SubGhzReceiver* subghz_txrx_get_receiver(SubGhzTxRx* instance); // TODO use only in DecodeRaw

//#############Create  new Key##############
bool subghz_txrx_gen_data_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit);

bool subghz_txrx_gen_data_protocol_and_te(
    SubGhzTxRx* instance,
    const char* preset_name,
    uint32_t frequency,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit,
    uint32_t te);

bool subghz_scene_set_type_submenu_gen_data_keeloq(
    SubGhzTxRx* txrx,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    const char* manufacture_name);

bool subghz_scene_set_type_submenu_gen_data_keeloq_bft( //TODO rename
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    uint32_t seed,
    const char* manufacture_name);

bool subghz_scene_set_type_submenu_gen_data_nice_flor( //TODO rename
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    bool nice_one);

bool subghz_scene_set_type_submenu_gen_data_faac_slh( //TODO rename
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    uint32_t seed,
    const char* manufacture_name);

bool subghz_scene_set_type_submenu_gen_data_alutech_at_4n( //TODO rename
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt);

bool subghz_scene_set_type_submenu_gen_data_somfy_telis( //TODO rename
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt);

bool subghz_txrx_gen_secplus_v2_protocol(
    SubGhzTxRx* txrx,
    const char* name_preset,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt);

bool subghz_txrx_gen_secplus_v1_protocol(
    SubGhzTxRx* txrx,
    const char* name_preset,
    uint32_t frequency);