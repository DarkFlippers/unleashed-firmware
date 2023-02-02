#include "nfc_types.h"

const char* nfc_get_dev_type(FuriHalNfcType type) {
    if(type == FuriHalNfcTypeA) {
        return "NFC-A";
    } else if(type == FuriHalNfcTypeB) {
        return "NFC-B";
    } else if(type == FuriHalNfcTypeF) {
        return "NFC-F";
    } else if(type == FuriHalNfcTypeV) {
        return "NFC-V";
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
    } else if(type == MfUltralightTypeNTAGI2C1K) {
        return "NTAG I2C 1K";
    } else if(type == MfUltralightTypeNTAGI2C2K) {
        return "NTAG I2C 2K";
    } else if(type == MfUltralightTypeNTAGI2CPlus1K) {
        return "NTAG I2C Plus 1K";
    } else if(type == MfUltralightTypeNTAGI2CPlus2K) {
        return "NTAG I2C Plus 2K";
    } else if(type == MfUltralightTypeNTAG203) {
        return "NTAG203";
    } else if(type == MfUltralightTypeUL11 && full_name) {
        return "Mifare Ultralight 11";
    } else if(type == MfUltralightTypeUL21 && full_name) {
        return "Mifare Ultralight 21";
    } else {
        return "Mifare Ultralight";
    }
}

const char* nfc_mf_classic_type(MfClassicType type) {
    if(type == MfClassicTypeMini) {
        return "Mifare Mini 0.3K";
    } else if(type == MfClassicType1k) {
        return "Mifare Classic 1K";
    } else if(type == MfClassicType4k) {
        return "Mifare Classic 4K";
    } else {
        return "Mifare Classic";
    }
}
