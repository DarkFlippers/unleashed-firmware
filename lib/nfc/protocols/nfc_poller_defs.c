#include "nfc_poller_defs.h"

#include <nfc/protocols/iso14443_3a/iso14443_3a_poller_defs.h>
#include <nfc/protocols/iso14443_3b/iso14443_3b_poller_defs.h>
#include <nfc/protocols/iso14443_4a/iso14443_4a_poller_defs.h>
#include <nfc/protocols/iso14443_4b/iso14443_4b_poller_defs.h>
#include <nfc/protocols/iso15693_3/iso15693_3_poller_defs.h>
#include <nfc/protocols/felica/felica_poller_defs.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight_poller_defs.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_defs.h>
#include <nfc/protocols/mf_plus/mf_plus_poller_defs.h>
#include <nfc/protocols/mf_desfire/mf_desfire_poller_defs.h>
#include <nfc/protocols/slix/slix_poller_defs.h>
#include <nfc/protocols/st25tb/st25tb_poller_defs.h>

const NfcPollerBase* nfc_pollers_api[NfcProtocolNum] = {
    [NfcProtocolIso14443_3a] = &nfc_poller_iso14443_3a,
    [NfcProtocolIso14443_3b] = &nfc_poller_iso14443_3b,
    [NfcProtocolIso14443_4a] = &nfc_poller_iso14443_4a,
    [NfcProtocolIso14443_4b] = &nfc_poller_iso14443_4b,
    [NfcProtocolIso15693_3] = &nfc_poller_iso15693_3,
    [NfcProtocolFelica] = &nfc_poller_felica,
    [NfcProtocolMfUltralight] = &mf_ultralight_poller,
    [NfcProtocolMfClassic] = &mf_classic_poller,
    [NfcProtocolMfPlus] = &mf_plus_poller,
    [NfcProtocolMfDesfire] = &mf_desfire_poller,
    [NfcProtocolSlix] = &nfc_poller_slix,
    /* Add new pollers here */
    [NfcProtocolSt25tb] = &nfc_poller_st25tb,
};
