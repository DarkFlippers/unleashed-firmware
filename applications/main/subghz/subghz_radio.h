#pragma once
//#include "subghz_i.h"
#include "helpers/subghz_types.h"
#include <lib/subghz/subghz_worker.h>
#include <lib/subghz/subghz_setting.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/protocols/raw.h>

#include "subghz_history.h"

typedef void (*SubGhzTxRxNeedSaveCallback)(void* context);

typedef struct SubGhzTxRx SubGhzTxRx;

SubGhzTxRx* subghz_txrx_alloc();
void subghz_txrx_free(SubGhzTxRx* txrx);
bool subghz_txrx_is_load_database(SubGhzTxRx* txrx);

void subghz_txrx_set_debug_pin_state(SubGhzTxRx* txrx, bool state);
bool subghz_txrx_get_debug_pin_state(SubGhzTxRx* txrx);

void subghz_set_preset(
    SubGhzTxRx* txrx,
    const char* preset_name,
    uint32_t frequency,
    uint8_t* preset_data,
    size_t preset_data_size);

const char* subghz_get_name_preset(SubGhzTxRx* txrx, const char* preset);
SubGhzRadioPreset subghz_get_preset(SubGhzTxRx* txrx);

void subghz_get_frequency_modulation(
    SubGhzTxRx* txrx,
    FuriString* frequency,
    FuriString* modulation,
    bool long_name);
bool subghz_tx_start(SubGhzTxRx* txrx, FlipperFormat* flipper_format);
//void subghz_rx_end(SubGhzTxRx* txrx); //depricated
void subghz_rx_start(SubGhzTxRx* txrx);
void subghz_txrx_stop(SubGhzTxRx* txrx);
void subghz_sleep(SubGhzTxRx* txrx);

SubGhzTxRxState subghz_txrx_get_state(SubGhzTxRx* txrx);

void subghz_hopper_update(SubGhzTxRx* txrx);
SubGhzHopperState subghz_hopper_get_state(SubGhzTxRx* txrx);
void subghz_hopper_set_state(SubGhzTxRx* txrx, SubGhzHopperState state);
void subghz_hopper_remove_pause(SubGhzTxRx* txrx);
void subghz_subghz_hopper_set_pause(SubGhzTxRx* txrx);

void subghz_speaker_on(SubGhzTxRx* txrx);
void subghz_speaker_off(SubGhzTxRx* txrx);
void subghz_speaker_mute(SubGhzTxRx* txrx);
void subghz_speaker_unmute(SubGhzTxRx* txrx);
void subghz_speaker_set_state(SubGhzTxRx* txrx, SubGhzSpeakerState state);
SubGhzSpeakerState subghz_speaker_get_state(SubGhzTxRx* txrx);
bool subghz_txrx_load_decoder_by_name_protocol(SubGhzTxRx* txrx, const char* name_protocol);
SubGhzProtocolDecoderBase* subghz_txrx_get_decoder(SubGhzTxRx* txrx);

void subghz_txrx_need_save_callback_set(
    SubGhzTxRx* txrx,
    SubGhzTxRxNeedSaveCallback callback,
    void* context);
FlipperFormat* subghz_txtx_get_fff_data(SubGhzTxRx* txrx);
SubGhzSetting* subghz_txrx_get_setting(SubGhzTxRx* txrx);

bool subghz_txrx_protocol_is_preserved(SubGhzTxRx* txrx);
bool subghz_txrx_protocol_is_send(SubGhzTxRx* txrx, bool check_type);

void subghz_txrx_receiver_set_filter(SubGhzTxRx* txrx, SubGhzProtocolFlag filter);

void subghz_txrx_set_rx_calback(SubGhzTxRx* txrx, SubGhzReceiverCallback callback, void* context);
void subghz_txrx_set_raw_file_encoder_worker_set_callback_end(
    SubGhzTxRx* txrx,
    SubGhzProtocolEncoderRAWCallbackEnd callback,
    void* context);

SubGhzReceiver* subghz_txrx_get_receiver(SubGhzTxRx* txrx); // TODO use only in DecodeRaw

//#############Create  new Key##############
bool subghz_gen_data_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit);

bool subghz_gen_data_protocol_and_te(
    SubGhzTxRx* txrx,
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

bool subghz_gen_secplus_v2_protocol(
    SubGhzTxRx* txrx,
    const char* name_preset,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt);

bool subghz_gen_secplus_v1_protocol(SubGhzTxRx* txrx, const char* name_preset, uint32_t frequency);