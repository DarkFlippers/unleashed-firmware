#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <m-string.h>
#include <storage/storage.h>

/** Get EMV application name by number
 * @param storage Storage instance
 * @param aid - AID number array
 * @param aid_len - AID length
 * @param aid_name - string to keep AID name
 * @return - true if AID found, false otherwies
 */
bool nfc_emv_parser_get_aid_name(
    Storage* storage,
    uint8_t* aid,
    uint8_t aid_len,
    string_t aid_name);

/** Get country name by country code
 * @param storage Storage instance
 * @param country_code - ISO 3166 country code
 * @param country_name - string to keep country name
 * @return - true if country found, false otherwies
 */
bool nfc_emv_parser_get_country_name(
    Storage* storage,
    uint16_t country_code,
    string_t country_name);

/** Get currency name by currency code
 * @param storage Storage instance
 * @param currency_code - ISO 3166 currency code
 * @param currency_name - string to keep currency name
 * @return - true if currency found, false otherwies
 */
bool nfc_emv_parser_get_currency_name(
    Storage* storage,
    uint16_t currency_code,
    string_t currency_name);
