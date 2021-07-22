#pragma once

#include "nfc_device.h"
#include <m-string.h>

uint16_t nfc_device_prepare_format_string(NfcDevice* dev, string_t format_string);
bool nfc_device_parse_format_string(NfcDevice* dev, string_t format_string);

uint16_t nfc_device_prepare_uid_string(NfcDevice* dev, string_t uid_string);
bool nfc_device_parse_uid_string(NfcDevice* dev, string_t uid_string);

uint16_t nfc_device_prepare_mifare_ul_string(NfcDevice* dev, string_t mifare_ul_string);
bool nfc_device_parse_mifare_ul_string(NfcDevice* dev, string_t mifare_ul_string);

uint16_t nfc_device_prepare_bank_card_string(NfcDevice* dev, string_t bank_card_string);
bool nfc_device_parse_bank_card_string(NfcDevice* dev, string_t bank_card_string);