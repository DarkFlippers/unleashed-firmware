#include "subghz_config_preset_custom.h"

#include <stdio.h>
#include <core/log.h>
#include <core/core_defines.h> // UNUSED()
#include <core/check.h> // furi_assert()
#include <math.h> // log2(), floor()

#include <flipper_format/flipper_format.h>

// https://www.ti.com/lit/ds/symlink/cc1101.pdf?ts=1671943815135
// page 35.
// 12 Data Rate Programming
//
#define DATARATE_FUNC_CHIP_FOSC 26000000.0 /* 26MHz */
#define DATARATE_FUNC_DIVIDER (1 << 28) /* 2 pow 28 */
#define DATARATE_FUNC_MULTIPLIER \
    (DATARATE_FUNC_CHIP_FOSC / DATARATE_FUNC_DIVIDER) /* should be 0.09685754 */

#define DATARATE_EXP_FORMULA_DIVISIBLE (1 << 20) /* 2 pow 20 */
#define DATARATE_EXP_FORMULA_MULTIPLIER \
    (DATARATE_EXP_FORMULA_DIVISIBLE / DATARATE_FUNC_CHIP_FOSC) /* should be 0.04032984 */

#define DATARATE_MNT_FORMULA_DIVISIBLE (1 << 28) /* 2 pow 28 */
#define DATARATE_MNT_FORMULA_MULTIPLIER \
    (DATARATE_MNT_FORMULA_DIVISIBLE / DATARATE_FUNC_CHIP_FOSC) /* should be 10.3244406 */
//

#define SUGHZ_CONFIG_TAG "SubGHz_Config"

uint8_t furi_hal_subghz_preset_ook_custom_async_regs[PRESET_OOK_CUSTOM_ADVANCED_AM_SIZE] = {0};

/** Check if cursom preset is AM (OOK) modulation
 *
 * This will check MOD_FORMAT bits in CC1101_MDMCFG2 register
 * If preset data doesn have this register - will return false.
 * This function will not fail in any case
 *
 * @param      preset_data  Custom preset data (registers and patable)
 * @param      data_len     Data length
 */
bool subghz_preset_custom_is_ook_modulation(const uint8_t* preset_data, uint8_t data_len) {
    if(preset_data != NULL) {
        for(uint8_t i = 2; i <= data_len; i += 2) {
            if(preset_data[i - 2] == CC1101_MDMCFG2) {
                return (preset_data[i - 1] & 0b01110000) == 0x30;
            }
        }
    }
    return false;
}

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
uint8_t subghz_preset_custom_get_bandwidth(const uint8_t* preset_data, uint8_t data_len) {
    if(preset_data != NULL) {
        for(uint8_t i = 2; i <= data_len; i += 2) {
            if(preset_data[i - 2] == CC1101_MDMCFG4) {
                return (preset_data[i - 1] & 0b11110000);
            }
        }
    }
    return CH_BANDWIDTH_INVALID;
}

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
bool subghz_preset_custom_set_bandwidth(uint8_t* preset_data, uint8_t data_len, uint8_t value) {
    if(preset_data != NULL) {
        for(uint8_t i = 2; i <= data_len; i += 2) {
            if(preset_data[i - 2] == CC1101_MDMCFG4) {
                preset_data[i - 1] = (preset_data[i - 1] & 0b00001111) | (0b11110000 & value);
                return true;
            }
        }
    }
    return false;
}

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
float subghz_preset_custom_get_datarate(const uint8_t* preset_data, uint8_t data_len) {
    if(preset_data != NULL) {
        uint8_t mantissa = 0xFF;
        uint8_t exponent = 0xFF; // Invalid, only 4 lower bits are singificant

        uint8_t step = 0;

        for(uint8_t i = 2; i <= data_len && step < 2; i += 2) {
            if(preset_data[i - 2] == CC1101_MDMCFG4) {
                exponent = preset_data[i - 1] & 0b00001111;
                step++;
            } else if(preset_data[i - 2] == CC1101_MDMCFG3) {
                mantissa = preset_data[i - 1];
                step++;
            }
        }

        if(step == 2) {
            return (float)((256 + mantissa) * (1 << exponent) * DATARATE_FUNC_MULTIPLIER);
        }
    }
    return -1;
}

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
bool subghz_preset_custom_set_datarate(uint8_t* preset_data, uint8_t data_len, float value) {
    if(preset_data != NULL) {
        uint8_t* pMantissa = NULL;
        uint8_t* pExponent = NULL;

        uint8_t step = 0;
        for(uint8_t i = 2; i <= data_len && step < 2; i += 2) {
            if(preset_data[i - 2] == CC1101_MDMCFG4) {
                pExponent = &preset_data[i - 1];
                step++;
            } else if(preset_data[i - 2] == CC1101_MDMCFG3) {
                pMantissa = &preset_data[i - 1];
                step++;
            }
        }

        // Has both registers in data - calculate values
        if(step == 2) {
            //            │      value * 2^20  │
            //  DRATE_E = │log2(──────────────)│
            //            └          Fosc      ┘

            double exponent = floor(log2(value * DATARATE_EXP_FORMULA_MULTIPLIER));
            uint8_t datarate_e = (uint8_t)exponent;

            //                 value * 2^28
            //  DRATE_M = (────────────────────) - 256
            //               Fosc *  2^DRATE_E
            double mantissa =
                floor((value * DATARATE_MNT_FORMULA_MULTIPLIER) / (1 << datarate_e) + 0.5) - 256;

            // If DRATE_M is rounded to the nearest integer and becomes 256, increment DRATE_E and use DRATE_M = 0.
            if(mantissa >= 256) {
                mantissa = 0;
                datarate_e += 1;
            }
            uint8_t datarate_m = (uint8_t)mantissa;

            *pExponent = (*pExponent & 0b11110000) | (datarate_e & 0b00001111);
            *pMantissa = datarate_m;

            return true;
        }
    }

    return false;
}

/** Print datarate value to string
 *
 * This is just convenience function
 *
 * @param      datarate datarate obtained from `subghz_preset_custom_get_datarate` function
 * @param      string   Target print buffer
 * @param      size     Target print buffer size
 */
void subghz_preset_custom_printf_datarate(float datarate, char* string, uint8_t size) {
    float kBaudRate = datarate / 1000.0f;
    snprintf(
        string,
        size,
        "%lu.%02lu kBd",
        (uint32_t)(kBaudRate), // decimal part
        (uint32_t)((kBaudRate - (uint32_t)kBaudRate) * 100) // fractional part multiplied by 100
    );
}

/** Get Manchester encoding/decoding flag value from preset data.
 *
 * This will get MANCHESTER_EN (3-rd) bit in CC1101_MDMCFG2 register
 * If CC1101_MDMCFG2 is not found in preset data - will return false
 *
 * @param      preset_data  Custom preset data (registers and patable)
 * @param      data_len     Data length
 */
bool subghz_preset_custom_get_machester_enable(const uint8_t* preset_data, uint8_t data_len) {
    if(preset_data != NULL) {
        for(uint8_t i = 2; i <= data_len; i += 2) {
            if(preset_data[i - 2] == CC1101_MDMCFG2) {
                return (preset_data[i - 1] & 0b00001000);
            }
        }
    }
    return false;
}

/** Set Manchester encoding/decoding flag value to preset data.
 *
 * This will set MANCHESTER_EN (3-rd) bit in CC1101_MDMCFG2 register
 * If CC1101_MDMCFG2 is not found in preset data - will return false
 *
 * @param      preset_data  Custom preset data (registers and patable)
 * @param      data_len     Data length
 */
bool subghz_preset_custom_set_machester_enable(uint8_t* preset_data, uint8_t data_len, bool value) {
    if(preset_data != NULL) {
        for(uint8_t i = 2; i <= data_len; i += 2) {
            if(preset_data[i - 2] == CC1101_MDMCFG2) {
                preset_data[i - 1] = (preset_data[i - 1] & 0b11110111) | (0b00001000 * value);
                return true;
            }
        }
    }
    return false;
}

/**
 * Initialize custom preset data
 */
void subghz_preset_custom_init_advanced_am_preset() {
    FURI_LOG_D(SUGHZ_CONFIG_TAG, "Initializing AM preset with custom Modem configuration");

    if(furi_hal_subghz_preset_ook_custom_async_regs[0]) {
        // already initialized
        FURI_LOG_D(SUGHZ_CONFIG_TAG, "Already initialized");
        return;
    }

    // Copy default AM270 preset
    memcpy(
        &furi_hal_subghz_preset_ook_custom_async_regs,
        &furi_hal_subghz_preset_ook_270khz_async_regs,
        sizeof(furi_hal_subghz_preset_ook_270khz_async_regs));

    const uint8_t ModemConfigStart = 4;

#if FURI_DEBUG
    const uint8_t ModemConfigEnd = ModemConfigStart + MODEM_CONFIG_REGISTERS_COUNT;
    for(uint8_t i = ModemConfigStart; i < ModemConfigEnd; ++i) {
        // Check we'll overwrite correct settings
        furi_assert(
            furi_hal_subghz_preset_ook_custom_async_regs[i * 2 + 0] ==
            furi_hal_subghz_custom_modulation_regs[i - ModemConfigStart][0]);
    }
#endif

    // Copy CUSTOM Modem preset
    memcpy(
        &furi_hal_subghz_preset_ook_custom_async_regs[ModemConfigStart * 2],
        &furi_hal_subghz_custom_modulation_regs,
        sizeof(furi_hal_subghz_custom_modulation_regs));

    // Copy default AM270 patable
    memcpy(
        &furi_hal_subghz_preset_ook_custom_async_regs[sizeof(
            furi_hal_subghz_preset_ook_270khz_async_regs)],
        &furi_hal_subghz_preset_ook_async_patable,
        sizeof(furi_hal_subghz_preset_ook_async_patable));

    // Here at the end we should have
    // <AM270 bytes> <CFGMDM regs> <AM270 bytes> 00 00 <AM270 patable>

#if FURI_DEBUG
    FURI_LOG_D(SUGHZ_CONFIG_TAG, "Custom OOK preset created");

    for(uint8_t i = 0; i < PRESET_OOK_CUSTOM_ADVANCED_AM_SIZE; i += 2) {
        FURI_LOG_D(
            SUGHZ_CONFIG_TAG,
            "Register: 0x%hhX, Value: 0x%hhX",
            furi_hal_subghz_preset_ook_custom_async_regs[i * 2 + 0],
            furi_hal_subghz_preset_ook_custom_async_regs[i * 2 + 1]);
    }
#endif

    FURI_LOG_D(SUGHZ_CONFIG_TAG, "Done");
}

/**
 * Create subghz preset file with custom am preset
 * this is used for preset initialization if subghz app
 */
FlipperFormat* subghz_preset_custom_advanced_am_preset_alloc() {
    FlipperFormat* advanced_am_preset = flipper_format_string_alloc();

    subghz_preset_custom_init_advanced_am_preset();

    flipper_format_write_hex(
        advanced_am_preset,
        (const char*)"Custom_preset_data",
        (const uint8_t*)&furi_hal_subghz_preset_ook_custom_async_regs[0],
        sizeof(furi_hal_subghz_preset_ook_custom_async_regs));

    flipper_format_rewind(advanced_am_preset);

    return advanced_am_preset;
}
