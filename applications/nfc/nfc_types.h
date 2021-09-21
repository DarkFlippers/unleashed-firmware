#pragma once

#include "st_errno.h"
#include "rfal_nfc.h"

#include <gui/view_dispatcher.h>
#include "nfc_worker.h"

static inline const char* nfc_get_rfal_type(rfalNfcDevType type) {
    if(type == RFAL_NFC_LISTEN_TYPE_NFCA) {
        return "NFC-A";
    } else if(type == RFAL_NFC_LISTEN_TYPE_NFCB) {
        return "NFC-B";
    } else if(type == RFAL_NFC_LISTEN_TYPE_NFCF) {
        return "NFC-F";
    } else if(type == RFAL_NFC_LISTEN_TYPE_NFCV) {
        return "NFC-V";
    } else if(type == RFAL_NFC_LISTEN_TYPE_ST25TB) {
        return "NFC-ST25TB";
    } else if(type == RFAL_NFC_LISTEN_TYPE_AP2P) {
        return "NFC-AP2P";
    } else {
        return "Unknown";
    }
}

static inline const char* nfc_get_dev_type(NfcDeviceType type) {
    if(type == NfcDeviceNfca) {
        return "NFC-A may be:";
    } else if(type == NfcDeviceNfcb) {
        return "NFC-B may be:";
    } else if(type == NfcDeviceNfcf) {
        return "NFC-F may be:";
    } else if(type == NfcDeviceNfcv) {
        return "NFC-V may be:";
    } else {
        return "Unknown";
    }
}

static inline const char* nfc_get_nfca_type(rfalNfcaListenDeviceType type) {
    if(type == RFAL_NFCA_T1T) {
        return "T1T";
    } else if(type == RFAL_NFCA_T2T) {
        return "T2T";
    } else if(type == RFAL_NFCA_T4T) {
        return "T4T";
    } else if(type == RFAL_NFCA_NFCDEP) {
        return "NFCDEP";
    } else if(type == RFAL_NFCA_T4T_NFCDEP) {
        return "T4T_NFCDEP";
    } else {
        return "Unknown";
    }
}

static inline const char* nfc_get_protocol(NfcProtocol protocol) {
    if(protocol == NfcDeviceProtocolEMV) {
        return "EMV bank card";
    } else if(protocol == NfcDeviceProtocolMifareUl) {
        return "Mifare Ultralight";
    } else {
        return "Unrecognized";
    }
}
