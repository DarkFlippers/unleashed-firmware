/**
 * @file nfc_supported_card_plugin.h
 * @brief Supported card plugin abstract interface.
 *
 * Supported card plugins are dynamically loaded libraries that help making sense of
 * a particular card's raw data, if a suitable plugin exists.
 *
 * For example, if some card serves as a bus ticket, instead of just displaying a data dump,
 * a suitable plugin will transform that data into a human-readable format, showing the number
 * of rides or balance left.
 * Because of the highly specialised nature of application-specific cards, a separate plugin
 * for each such card type must be implemented.
 *
 * To add a new plugin, create a uniquely-named .c file in the `supported_cards` directory
 * and implement at least the parse() function in the NfcSupportedCardsPlugin structure.
 * Then, register the plugin in the `application.fam` file in the `nfc` directory. Use the existing
 * entries as an example. After being registered, the plugin will be automatically deployed with the application.
 *
 * @note the APPID field MUST end with `_parser` so the applicaton would know that this particular file
 * is a supported card plugin.
 *
 * @see nfc_supported_cards.h
 */
#pragma once

#include <furi/core/string.h>

#include <nfc/nfc.h>
#include <nfc/nfc_device.h>

/**
 * @brief Unique string identifier for supported card plugins.
 */
#define NFC_SUPPORTED_CARD_PLUGIN_APP_ID "NfcSupportedCardPlugin"

/**
 * @brief Currently supported plugin API version.
 */
#define NFC_SUPPORTED_CARD_PLUGIN_API_VERSION 1

/**
 * @brief Verify that the card is of a supported type.
 *
 * This function should be implemented if a quick check exists
 * allowing to verify that the plugin is working with the appropriate card type.
 * Such checks may include, but are not limited to: reading a specific sector,
 * performing a certain read operation, etc.
 *
 * @param[in,out] nfc pointer to an Nfc instance.
 * @returns true if the card was successfully verified, false otherwise.
 */
typedef bool (*NfcSupportedCardPluginVerify)(Nfc* nfc);

/**
 * @brief Read the card using a custom procedure.
 *
 * This function should be implemented if a card requires some special reading
 * procedure not covered in the vanilla poller. Examples include, but are not
 * limited to: reading with particular security keys, mandatory order of read
 * operations, etc.
 *
 * @param[in,out] nfc pointer to an Nfc instance.
 * @param[in,out] device pointer to a device instance to hold the read data.
 * @returns true if the card was successfully read, false otherwise.
 */
typedef bool (*NfcSupportedCardPluginRead)(Nfc* nfc, NfcDevice* device);

/**
 * @brief Parse raw data into human-readable representation.
 *
 * A supported card plugin may contain only this function, if no special verification
 * or reading procedures are not required. In any case, the data must be complete and
 * available through the `device` parameter at the time of calling.
 *
 * The output format is free and application-dependent. Multiple lines should
 * be separated by newline character.
 *
 * @param[in] device pointer to a device instance holding the data is to be parsed.
 * @param[out] parsed_data pointer to the string to contain the formatted result.
 * @returns true if the card was successfully parsed, false otherwise.
 */
typedef bool (*NfcSupportedCardPluginParse)(const NfcDevice* device, FuriString* parsed_data);

/**
 * @brief Supported card plugin interface.
 *
 * For a minimally functional plugin, only the parse() function must be implemented.
 */
typedef struct {
    NfcProtocol protocol; /**< Identifier of the protocol this card type works on top of. */
    NfcSupportedCardPluginVerify verify; /**< Pointer to the verify() function. */
    NfcSupportedCardPluginRead read; /**< Pointer to the read() function. */
    NfcSupportedCardPluginParse parse; /**< Pointer to the parse() function. */
} NfcSupportedCardsPlugin;
