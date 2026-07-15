#pragma once

#include <storage/storage.h>
#include <furi.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Look up a country's 2-letter code (ISO 3166-1 Alpha-2).
 * 
 * @param storage       Pointer to a valid Storage.
 * @param country_code  Numeric ISO3166 code. e.g., 4 for Afghanistan.
 * @param out_two_letter A caller-allocated FuriString to fill with the 2-letter code, if found.
 */
bool iso_3166_get_two_letter(Storage* storage, uint16_t country_code, FuriString* out_two_letter);

/**
 * @brief Look up a country's 3-letter code (ISO 3166-1 Alpha-3).
 * 
 * @param storage       Pointer to a valid Storage.
 * @param country_code  Numeric ISO3166 code.
 * @param out_three_letter A caller-allocated FuriString to fill with the 3-letter code, if found.
 */
bool iso_3166_get_three_letter(
    Storage* storage,
    uint16_t country_code,
    FuriString* out_three_letter);

/**
 * @brief Look up a country's full name.
 * 
 * @param storage       Pointer to a valid Storage.
 * @param country_code  Numeric ISO3166 code.
 * @param out_full_name A caller-allocated FuriString to fill with the country's full name.
 */
bool iso_3166_get_full_name(Storage* storage, uint16_t country_code, FuriString* out_full_name);

#ifdef __cplusplus
}
#endif
