#include "nfc_listener_defs.h"

#include <nfc/protocols/iso14443_3a/iso14443_3a_listener_defs.h>
#include <nfc/protocols/iso14443_4a/iso14443_4a_listener_defs.h>
#include <nfc/protocols/iso15693_3/iso15693_3_listener_defs.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight_listener_defs.h>
#include <nfc/protocols/mf_classic/mf_classic_listener_defs.h>
#include <nfc/protocols/slix/slix_listener_defs.h>
#include <nfc/protocols/felica/felica_listener_defs.h>

const NfcListenerBase* nfc_listeners_api[NfcProtocolNum] = {
    [NfcProtocolIso14443_3a] = &nfc_listener_iso14443_3a,
    [NfcProtocolIso14443_3b] = NULL,
    [NfcProtocolIso14443_4a] = &nfc_listener_iso14443_4a,
    [NfcProtocolIso14443_4b] = NULL,
    [NfcProtocolIso15693_3] = &nfc_listener_iso15693_3,
    [NfcProtocolMfUltralight] = &mf_ultralight_listener,
    [NfcProtocolMfClassic] = &mf_classic_listener,
    [NfcProtocolMfDesfire] = NULL,
    [NfcProtocolSlix] = &nfc_listener_slix,
    [NfcProtocolSt25tb] = NULL,
    [NfcProtocolFelica] = &nfc_listener_felica,
};
