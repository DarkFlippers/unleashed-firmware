#pragma once
#include "subghz_types.h"
#include "subghz_txrx.h"

/**
 * Generate data for protocol
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param preset_name Name of preset
 * @param frequency Frequency in Hz
 * @param protocol_name Name of protocol
 * @param key Key
 * @param bit Bit
 * @return bool True if success
 */
bool subghz_txrx_gen_data_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit);

/**
 * Generate data for protocol and te
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param preset_name Name of preset
 * @param frequency Frequency in Hz
 * @param protocol_name Name of protocol
 * @param key Key
 * @param bit Bit
 * @param te Te
 * @return bool True if success
 */
bool subghz_txrx_gen_data_protocol_and_te(
    SubGhzTxRx* instance,
    const char* preset_name,
    uint32_t frequency,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit,
    uint32_t te);

/**
 * Generate data Keeloq protocol
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param preset_name Name of preset
 * @param frequency Frequency in Hz
 * @param serial Serial number
 * @param btn Button
 * @param cnt Counter
 * @param manufacture_name Name of Keeloq sysmem
 * @return bool True if success
 */
bool subghz_txrx_gen_keeloq_protocol(
    SubGhzTxRx* instance,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    const char* manufacture_name);

bool subghz_txrx_gen_keeloq_bft_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    uint32_t seed,
    const char* manufacture_name);

bool subghz_txrx_gen_nice_flor_s_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    bool nice_one);

bool subghz_txrx_gen_faac_slh_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt,
    uint32_t seed,
    const char* manufacture_name);

bool subghz_txrx_gen_alutech_at_4n_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt);

bool subghz_txrx_gen_somfy_telis_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt);

bool subghz_txrx_gen_came_atomo_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint16_t cnt);

/**
 * Generate data SecPlus v2 protocol
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param name_preset Name of preset
 * @param frequency Frequency in Hz
 * @param serial Serial number
 * @param btn Button
 * @param cnt Counter
 * @return bool True if success
 */
bool subghz_txrx_gen_secplus_v2_protocol(
    SubGhzTxRx* instance,
    const char* name_preset,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt);

/**
 * Generate data SecPlus v1 protocol
 * 
 * @param instance Pointer to a SubGhzTxRx
 * @param name_preset Name of preset
 * @param frequency Frequency in Hz
 * @return bool True if success
 */
bool subghz_txrx_gen_secplus_v1_protocol(
    SubGhzTxRx* instance,
    const char* name_preset,
    uint32_t frequency);