/**
 * @file nfc_protocol_support_defs.c
 * @brief Application-level protocol support definitions.
 *
 * This file is to be modified whenever support for
 * a new protocol is to be added.
 */
#include "nfc_protocol_support_base.h"

#include <nfc/protocols/nfc_protocol.h>

#include "iso14443_3a/iso14443_3a.h"
#include "iso14443_3b/iso14443_3b.h"
#include "iso14443_4a/iso14443_4a.h"
#include "iso14443_4b/iso14443_4b.h"
#include "iso15693_3/iso15693_3.h"
#include "felica/felica.h"
#include "mf_ultralight/mf_ultralight.h"
#include "mf_classic/mf_classic.h"
#include "mf_plus/mf_plus.h"
#include "mf_desfire/mf_desfire.h"
#include "slix/slix.h"
#include "st25tb/st25tb.h"

/**
 * @brief Array of pointers to concrete protocol support implementations.
 *
 * When adding support for a new protocol, add it to the end of this array
 * under its respective index.
 *
 * @see nfc_protocol.h
 */
const NfcProtocolSupportBase* nfc_protocol_support[NfcProtocolNum] = {
    [NfcProtocolIso14443_3a] = &nfc_protocol_support_iso14443_3a,
    [NfcProtocolIso14443_3b] = &nfc_protocol_support_iso14443_3b,
    [NfcProtocolIso14443_4a] = &nfc_protocol_support_iso14443_4a,
    [NfcProtocolIso14443_4b] = &nfc_protocol_support_iso14443_4b,
    [NfcProtocolIso15693_3] = &nfc_protocol_support_iso15693_3,
    [NfcProtocolFelica] = &nfc_protocol_support_felica,
    [NfcProtocolMfUltralight] = &nfc_protocol_support_mf_ultralight,
    [NfcProtocolMfClassic] = &nfc_protocol_support_mf_classic,
    [NfcProtocolMfPlus] = &nfc_protocol_support_mf_plus,
    [NfcProtocolMfDesfire] = &nfc_protocol_support_mf_desfire,
    [NfcProtocolSlix] = &nfc_protocol_support_slix,
    [NfcProtocolSt25tb] = &nfc_protocol_support_st25tb,
    /* Add new protocol support implementations here */
};
