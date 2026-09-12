#pragma once

#include "../types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderSecPlus_v2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 32 bit
 * @param btn Button number, 8 bit
 * @param cnt Container value, 28 bit
 * @param manufacture_name Name of manufacturer's key
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_secplus_v2_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt,
    SubGhzRadioPreset* preset);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderKeeloq instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 28 bit
 * @param btn Button number, 4 bit
 * @param cnt Counter value, 16 bit
 * @param manufacture_name Name of manufacturer's key
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_keeloq_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    const char* manufacture_name,
    SubGhzRadioPreset* preset);

/**
 * Key generation for BFT.
 * @param context Pointer to a SubGhzProtocolEncoderKeeloq instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 28 bit
 * @param btn Button number, 4 bit
 * @param cnt Counter value, 16 bit
 * @param seed Seed value, 32 bit
 * @param manufacture_name Name of manufacturer's key
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_keeloq_seed_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    uint32_t seed,
    const char* manufacture_name,
    SubGhzRadioPreset* preset);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderFaacSLH instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 28 bit
 * @param btn Button number, 4 bit
 * @param cnt Counter value, 16 bit
 * @param seed Seed value, 32 bit
 * @param manufacture_name Name of manufacturer's key
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_faac_slh_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt,
    uint32_t seed,
    const char* manufacture_name,
    SubGhzRadioPreset* preset);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderAlutech_at_4n instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 24 bit
 * @param btn Button number, 8 bit
 * @param cnt Counter value, 16 bit
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_alutech_at_4n_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderCameAtomo instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 24 bit
 * @param cnt Counter value, 16 bit
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_came_atomo_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint16_t cnt,
    SubGhzRadioPreset* preset);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderPhoenix_V2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number
 * @param cnt Counter value, 16 bit
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_phoenix_v2_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint16_t cnt,
    SubGhzRadioPreset* preset);

/**
 * New remote generation.
 * @param context Pointer to a SubGhzProtocolEncoderNiceFlorS instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number
 * @param btn Button number, 4 bit
 * @param cnt Counter value, 16 bit
 * @param preset Modulation, SubGhzRadioPreset
 * @param nice_one Nice One if true, Nice Flor S if false
 * @return true On success
 */
bool subghz_protocol_nice_flor_s_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset,
    bool nice_one);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderSomfyTelis instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 24 bit
 * @param btn Button number, 8 bit
 * @param cnt Counter value, 16 bit
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_somfy_telis_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderSomfyKeytis instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 24 bit
 * @param btn Button number, 8 bit
 * @param cnt Counter value, 16 bit
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_somfy_keytis_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderKingGates_stylo_4k instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 24 bit
 * @param btn Button number, 8 bit
 * @param cnt Counter value, 16 bit
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_kinggates_stylo_4k_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderBenincaARC instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 32 bit
 * @param btn Button number, 8 bit
 * @param cnt Counter value, 32 bit
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_beninca_arc_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt,
    SubGhzRadioPreset* preset);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderJarolift instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 24 bit
 * @param btn Button number, 8 bit
 * @param cnt Counter value, 16 bit
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_jarolift_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderDitecGOL4 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number
 * @param btn Button number, 8 bit
 * @param cnt Counter value, 16 bit
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_ditec_gol4_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset);

typedef struct SubGhzProtocolDecoderBinRAW SubGhzProtocolDecoderBinRAW;

void subghz_protocol_decoder_bin_raw_data_input_rssi(
    SubGhzProtocolDecoderBinRAW* instance,
    float rssi);

/**
 * Validation of fixed parts SubGhzProtocolDecoderSecPlus_v1.
 * @param fixed fixed parts
 * @return true On success
 */
bool subghz_protocol_secplus_v1_check_fixed(uint32_t fixed);

/**
 * Calculate CRC8 for Marantec protocol.
 * @param data Pointer to the data buffer
 * @param len Length of the data buffer
 * @return CRC8 value
 */
uint8_t subghz_protocol_marantec_crc8(uint8_t* data, size_t len);

/**
 * Per-parcel obfuscation mask used by Nice O-Code.
 * @param n Parcel index 0..15
 * @param mask_32 Mask applied to the 28 bit serial (may be NULL)
 * @param mask_16 Mask applied to the installer code (may be NULL)
 */
void subghz_protocol_nice_o_mask(uint8_t n, uint32_t* mask_32, uint16_t* mask_16);

/**
 * Recover the parcel index carried by a Nice O-Code packet.
 */
uint8_t subghz_protocol_nice_o_get_parcel(uint64_t data);

/**
 * Choose whether the Nice Flor-S decoder hides O-Code frames.
 * With this on, a 52 bit frame is reported only once a second frame of the same press
 * confirms it is plain Flor-S; O-Code frames, which cannot be decoded without the
 * remote's installer code, are dropped instead of surfacing with a random serial.
 * On by default. An app that wants the raw frames, such as an installer code finder,
 * turns it off.
 * @param context Decoder instance, from subghz_receiver_search_decoder_base_by_name()
 * @param skip true to hide O-Code frames
 */
void subghz_protocol_decoder_nice_flor_s_set_skip_o_code(void* context, bool skip);

/** Returned by the Nice Flor-S cipher when the rainbow table cannot be read. */
#define SUBGHZ_NO_NICE_FLOR_S_RAINBOW_TABLE 0

/**
 * Nice Flor-S cipher parameterised with a 16 bit installer code (Nice O-Code).
 * Passing ic == 0xFFFF reproduces plain Nice Flor-S.
 * The rainbow table is read from the SD card on first use and kept, so calling this
 * in a loop costs one card read, not one per call.
 * @param data Captured 52/72 bit key
 * @param ic Installer code, already XORed with the parcel mask
 * @param file_name Full path to the rainbow table
 */
uint64_t subghz_protocol_nice_flor_s_decrypt_ic(uint64_t data, uint16_t ic, const char* file_name);

#ifdef __cplusplus
}
#endif
