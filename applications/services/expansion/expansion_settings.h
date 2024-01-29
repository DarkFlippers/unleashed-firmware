/**
 * @file expansion_settings.h
 * @brief Expansion module support settings.
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Expansion module support settings storage type.
 */
typedef struct {
    /**
     * Numerical index of serial port used to communicate
     * with expansion modules.
     */
    uint8_t uart_index;
} ExpansionSettings;

/**
 * @brief Load expansion module support settings from file.
 *
 * @param[out] settings pointer to an ExpansionSettings instance to load settings into.
 * @returns true if the settings were successfully loaded, false otherwise.
 */
bool expansion_settings_load(ExpansionSettings* settings);

/**
 * @brief Save expansion module support settings to file.
 *
 * @param[in] settings pointer to an ExpansionSettings instance to save settings from.
 * @returns true if the settings were successfully saved, false otherwise.
 */
bool expansion_settings_save(ExpansionSettings* settings);

#ifdef __cplusplus
}
#endif
