#pragma once

#include <furi_hal_subghz_configs.h>
#include <string.h> /* memcpy() */

#define ADVANCED_AM_PRESET_NAME "AM*"

// Awailable bandwidth values
// Setup in MDMCFG4 register
#define CH_BANDWIDTH_058 0b11110000
#define CH_BANDWIDTH_068 0b11100000
#define CH_BANDWIDTH_081 0b11010000
#define CH_BANDWIDTH_102 0b11000000

#define CH_BANDWIDTH_116 0b10110000
#define CH_BANDWIDTH_135 0b10100000
#define CH_BANDWIDTH_162 0b10010000
#define CH_BANDWIDTH_203 0b10000000

#define CH_BANDWIDTH_232 0b01110000
#define CH_BANDWIDTH_270 0b01100000
#define CH_BANDWIDTH_325 0b01010000
#define CH_BANDWIDTH_406 0b01000000

#define CH_BANDWIDTH_464 0b00110000
#define CH_BANDWIDTH_541 0b00100000
#define CH_BANDWIDTH_650 0b00010000
#define CH_BANDWIDTH_812 0b00000000

#define CH_BANDWIDTH_INVALID 0xFF

static const uint8_t subghz_preset_custom_bandwidth_values[] = {
    CH_BANDWIDTH_058,
    CH_BANDWIDTH_068,
    CH_BANDWIDTH_081,
    CH_BANDWIDTH_102,

    CH_BANDWIDTH_116,
    CH_BANDWIDTH_135,
    CH_BANDWIDTH_162,
    CH_BANDWIDTH_203,

    CH_BANDWIDTH_232,
    CH_BANDWIDTH_270,
    CH_BANDWIDTH_325,
    CH_BANDWIDTH_406,

    CH_BANDWIDTH_464,
    CH_BANDWIDTH_541,
    CH_BANDWIDTH_650,
    CH_BANDWIDTH_812,
};
#define CH_BANDWIDTH_NUM (sizeof(subghz_preset_custom_bandwidth_values) / sizeof(uint8_t))

#define DATARATE_EXPONENT_3_79_kBaud 0b00000111 // 7
#define DATARATE_MANTISSA_3_79_kBaud 0x32

#define CHANNEL_SPACING_25_EXPONENT 0b00000000 /* last bit */
#define CHANNEL_SPACING_25_MANTISSA 0x00

#define MODEM_CONFIG_REGISTERS_COUNT 5
#define PRESET_OOK_CUSTOM_ADVANCED_AM_SIZE                 \
    sizeof(furi_hal_subghz_preset_ook_270khz_async_regs) + \
        sizeof(furi_hal_subghz_preset_ook_async_patable)

extern uint8_t furi_hal_subghz_preset_ook_custom_async_regs[PRESET_OOK_CUSTOM_ADVANCED_AM_SIZE];

static const uint8_t furi_hal_subghz_custom_modulation_regs[MODEM_CONFIG_REGISTERS_COUNT][2] = {
    // Channel spacing is 25kHz, no Forward Error Correction, 2 preamble bytes (will be ignored)
    {CC1101_MDMCFG0, CHANNEL_SPACING_25_MANTISSA},
    {CC1101_MDMCFG1, 0x00 | CHANNEL_SPACING_25_EXPONENT},

    // [0:2] SYNC_MODE      = 00    // No preamble/sync
    // [3:3] MANCHESTER_EN  = 0     // Disable
    // [4:6] MOD_FORMAT     = 03    // Format ASK/OOK
    // [7:7] DEM_DCFILT_OFF = 0     // Enable
    {CC1101_MDMCFG2, 0x30},

    // 3.79 kBaud data rate (mantissa in 3rd register)
    {CC1101_MDMCFG3, DATARATE_MANTISSA_3_79_kBaud},

    // 270.8333 kHz Rx BW filer (hi) and 3.79 kBaud data rate (exponent in 4th register)
    {CC1101_MDMCFG4, DATARATE_EXPONENT_3_79_kBaud | CH_BANDWIDTH_270},
};

#ifdef __cplusplus
extern "C" {
#endif

/** Check if cursom preset is AM (OOK) modulation
 *
 * This will check MOD_FORMAT bits in CC1101_MDMCFG2 register
 * If preset data doesn have this register - will return false.
 * This function will not fail in any case
 *
 * @param      preset_data  Custom preset data (registers and patable)
 * @param      data_len     Data length
 */
bool subghz_preset_custom_is_ook_modulation(const uint8_t* preset_data, uint8_t data_len);

/** Get bandwidth value from preset data.
 *
 * This will get HIGHER bits in CC1101_MDMCFG4 register
 * If CC1101_MDMCFG4 is not found in preset data - will return
 * CH_BANDWIDTH_INVALID (0xFF)
 * If there is ANY low 4 bits in returned value - the value is invalid
 *
 * @param      preset_data  Custom preset data (registers and patable)
 * @param      data_len     Data length
 */
uint8_t subghz_preset_custom_get_bandwidth(const uint8_t* preset_data, uint8_t data_len);

/** Set bandwidth value to preset data.
 *
 * This will set HIGHER bits in CC1101_MDMCFG4 register
 * If CC1101_MDMCFG4 is not found in preset data - will do nothing and return false
 * If there are ANY low 4 bits in provided value - they will be ignored
 *
 * @param      preset_data  Custom preset data (registers and patable)
 * @param      data_len     Data length
 * @param      value        New bandwidth value. See macros definition for possible values
 */
bool subghz_preset_custom_set_bandwidth(uint8_t* preset_data, uint8_t data_len, uint8_t value);

/** Get data rate value from preset data.
 *
 * This will get DRATE_M and DRATE_E bits from CC1101_MDMCFG3 and CC1101_MDMCFG4 registers
 * and calculate the value for 26MHz chip oscillator by formula from datasheet.
 *
 * If CC1101_MDMCFG[3:4] are not found in preset data - will return `-1`
 *
 * @param      preset_data  Custom preset data (registers and patable)
 * @param      data_len     Data length
 */
float subghz_preset_custom_get_datarate(const uint8_t* preset_data, uint8_t data_len);

/** Set data rate value to preset data.
 *
 * This will update DRATE_M and DRATE_E bits from CC1101_MDMCFG3 and CC1101_MDMCFG4 registers
 * with calculated values for 26MHz chip oscillator by formula from datasheet.
 *
 * If CC1101_MDMCFG[3:4] are not found in preset data - will return false
 *
 * @param      preset_data  Custom preset data (registers and patable)
 * @param      data_len     Data length
 * @param      value        value in kBaud
 */
bool subghz_preset_custom_set_datarate(uint8_t* preset_data, uint8_t data_len, float value);

/** Print datarate value to string
 *
 * This is just conviniece function
 *
 * @param      datarate datarate obtained from `subghz_preset_custom_get_datarate` function
 * @param      string   Target print buffer
 * @param      size     Target print buffer size
 */
void subghz_preset_custom_printf_datarate(float datarate, char* string, uint8_t size);

/** Get Manchester encoding/decoding flag value from preset data.
 *
 * This will get MANCHESTER_EN (3-rd) bit in CC1101_MDMCFG2 register
 * If CC1101_MDMCFG2 is not found in preset data - will return false
 *
 * @param      preset_data  Custom preset data (registers and patable)
 * @param      data_len     Data length
 */
bool subghz_preset_custom_get_machester_enable(const uint8_t* preset_data, uint8_t data_len);

/** Set Manchester encoding/decoding flag value to preset data.
 *
 * This will set MANCHESTER_EN (3-rd) bit in CC1101_MDMCFG2 register
 * If CC1101_MDMCFG2 is not found in preset data - will return false
 *
 * @param      preset_data  Custom preset data (registers and patable)
 * @param      data_len     Data length
 */
bool subghz_preset_custom_set_machester_enable(uint8_t* preset_data, uint8_t data_len, bool value);

/**
 * Initialize advanced am custom preset
 */
void subghz_preset_custom_init_advanced_am_preset();

/**
 * Create subghz preset file with custom am preset
 * this is used for preset initialization if subghz app
 */
struct FlipperFormat* subghz_preset_custom_advanced_am_preset_alloc();

#ifdef __cplusplus
}
#endif