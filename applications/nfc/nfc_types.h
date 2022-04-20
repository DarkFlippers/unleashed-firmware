#pragma once

#include "nfc_device.h"

const char* nfc_get_dev_type(FuriHalNfcType type);

const char* nfc_guess_protocol(NfcProtocol protocol);

const char* nfc_mf_ul_type(MfUltralightType type, bool full_name);

const char* nfc_mf_classic_type(MfClassicType type);
