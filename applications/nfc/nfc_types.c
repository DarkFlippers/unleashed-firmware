#include "nfc_types.h"

const char* nfc_get_rfal_type(rfalNfcDevType type) {
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

const char* nfc_get_dev_type(NfcDeviceType type) {
    if(type == NfcDeviceNfca) {
        return "NFC-A";
    } else if(type == NfcDeviceNfcb) {
        return "NFC-B";
    } else if(type == NfcDeviceNfcf) {
        return "NFC-F";
    } else if(type == NfcDeviceNfcv) {
        return "NFC-V";
    } else {
        return "Unknown";
    }
}

const char* nfc_get_nfca_type(rfalNfcaListenDeviceType type) {
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

const char* nfc_guess_protocol(NfcProtocol protocol) {
    if(protocol == NfcDeviceProtocolEMV) {
        return "EMV bank card";
    } else if(protocol == NfcDeviceProtocolMifareUl) {
        return "Mifare Ultral/NTAG";
    } else if(protocol == NfcDeviceProtocolMifareClassic) {
        return "Mifare Classic";
    } else if(protocol == NfcDeviceProtocolMifareDesfire) {
        return "Mifare DESFire";
    } else {
        return "Unrecognized";
    }
}

const char* nfc_mf_ul_type(MfUltralightType type, bool full_name) {
    if(type == MfUltralightTypeNTAG213) {
        return "NTAG213";
    } else if(type == MfUltralightTypeNTAG215) {
        return "NTAG215";
    } else if(type == MfUltralightTypeNTAG216) {
        return "NTAG216";
    } else if(type == MfUltralightTypeUL11 && full_name) {
        return "Mifare Ultralight 11";
    } else if(type == MfUltralightTypeUL21 && full_name) {
        return "Mifare Ultralight 21";
    } else {
        return "Mifare Ultralight";
    }
}

const char* nfc_mf_classic_type(MfClassicType type) {
    if(type == MfClassicType1k) {
        return "Mifare Classic 1K";
    } else if(type == MfClassicType4k) {
        return "Mifare Classic 4K";
    } else {
        return "Mifare Classic";
    }
}
