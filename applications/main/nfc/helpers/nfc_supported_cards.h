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
 * @brief NfcSupportedCards opaque type definition.
 */
typedef struct NfcSupportedCards NfcSupportedCards;

/**
 * @brief Allocate NfcSupportedCards instance.
 *
 * @return pointer to allocated NfcSupportedCards instance.
 */
NfcSupportedCards* nfc_supported_cards_alloc(void);

/**
 * @brief Delete an NfcSupportedCards instance
 * 
 * @param[in] instance pointer to instance to be deleted.
 */
void nfc_supported_cards_free(NfcSupportedCards* instance);

/**
 * @brief Load plugins information to cache.
 *
 * @note This function must be called before calling read and parse fanctions.
 *
 * @param[in, out] instance pointer to NfcSupportedCards instance.
 */
void nfc_supported_cards_load_cache(NfcSupportedCards* instance);

/**
 * @brief Read the card using a custom procedure.
 *
 * This function will load all suitable supported card plugins one by one and
 * try to execute the custom read procedure specified in each. Upon first success,
 * no further attempts will be made and the function will return.
 *
 * @param[in, out] instance pointer to NfcSupportedCards instance.
 * @param[in,out] device pointer to a device instance to hold the read data.
 * @param[in,out] nfc pointer to an Nfc instance.
 * @returns true if the card was successfully read, false otherwise.
 *
 * @see NfcSupportedCardPluginRead for detailed description.
 */
bool nfc_supported_cards_read(NfcSupportedCards* instance, NfcDevice* device, Nfc* nfc);

/**
 * @brief Parse raw data into human-readable representation.
 *
 * This function will load all suitable supported card plugins one by one and
 * try to parse the data according to each implementation. Upon first success,
 * no further attempts will be made and the function will return.
 *
 * @param[in, out] instance pointer to NfcSupportedCards instance.
 * @param[in] device pointer to a device instance holding the data is to be parsed.
 * @param[out] parsed_data pointer to the string to contain the formatted result.
 * @returns true if the card was successfully parsed, false otherwise.
 *
 * @see NfcSupportedCardPluginParse for detailed description.
 */
bool nfc_supported_cards_parse(
    NfcSupportedCards* instance,
    NfcDevice* device,
    FuriString* parsed_data);

#ifdef __cplusplus
}
#endif
