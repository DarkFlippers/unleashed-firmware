/**
 * @file nfc_supported_cards.h
 * @brief Supported card plugin loader interface.
 *
 * @see nfc_supported_card_plugin.h for instructions on adding a new plugin.
 */
#pragma once

#include <core/string.h>

#include <nfc/nfc.h>
#include <nfc/nfc_device.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read the card using a custom procedure.
 *
 * This function will load all suitable supported card plugins one by one and
 * try to execute the custom read procedure specified in each. Upon first success,
 * no further attempts will be made and the function will return.
 *
 * @param[in,out] device pointer to a device instance to hold the read data.
 * @param[in,out] nfc pointer to an Nfc instance.
 * @returns true if the card was successfully read, false otherwise.
 *
 * @see NfcSupportedCardPluginRead for detailed description.
 */
bool nfc_supported_cards_read(NfcDevice* device, Nfc* nfc);

/**
 * @brief Parse raw data into human-readable representation.
 *
 * This function will load all suitable supported card plugins one by one and
 * try to parse the data according to each implementation. Upon first success,
 * no further attempts will be made and the function will return.
 *
 * @param[in] device pointer to a device instance holding the data is to be parsed.
 * @param[out] parsed_data pointer to the string to contain the formatted result.
 * @returns true if the card was successfully parsed, false otherwise.
 *
 * @see NfcSupportedCardPluginParse for detailed description.
 */
bool nfc_supported_cards_parse(const NfcDevice* device, FuriString* parsed_data);

#ifdef __cplusplus
}
#endif
