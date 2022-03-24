#pragma once

#include "st_errno.h"
#include "rfal_nfc.h"

#include <gui/view_dispatcher.h>
#include "nfc_worker.h"

const char* nfc_get_rfal_type(rfalNfcDevType type);

const char* nfc_get_dev_type(NfcDeviceType type);

const char* nfc_get_nfca_type(rfalNfcaListenDeviceType type);

const char* nfc_guess_protocol(NfcProtocol protocol);

const char* nfc_mf_ul_type(MfUltralightType type, bool full_name);

const char* nfc_mf_classic_type(MfClassicType type);
