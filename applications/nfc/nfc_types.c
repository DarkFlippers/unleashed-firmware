#include "nfc_types.h"

const char* nfc_get_dev_type(FuriHalNfcType type) {
    switch(type){
        case FuriHalNfcTypeA:
            return "NFC-A";
        case FuriHalNfcTypeB:
            return "NFC-B";
        case FuriHalNfcTypeF:
            return "NFC-F";
        case FuriHalNfcTypeV:
            return "NFC-V";
        default:
            return "Unknown";
    }
}

const char* nfc_guess_protocol(NfcProtocol protocol) {
    switch (protocol) {
        case NfcDeviceProtocolEMV:
            return "EMV bank card";
        case NfcDeviceProtocolMifareUl:
            return "Mifare Ultral/NTAG";
        case NfcDeviceProtocolMifareClassic:
            return "Mifare Classic";
        case NfcDeviceProtocolMifareDesfire:
            return "Mifare DESFire";
        default:
            return "Unrecognized";
    }
}

const char* nfc_mf_ul_type(MfUltralightType type, bool full_name) {
    if(type == MfUltralightTypeUL11 && full_name) {
        return "Mifare Ultralight 11";
    } else if(type == MfUltralightTypeUL21 && full_name) {
        return "Mifare Ultralight 21";
    } else {
    switch (type) {
        case MfUltralightTypeNTAG213:
            return "NTAG213";
        case MfUltralightTypeNTAG215:
            return "NTAG215";
        case MfUltralightTypeNTAG216:
            return "NTAG216";
        case MfUltralightTypeNTAGI2C1K:
            return "NTAG I2C 1K";
        case MfUltralightTypeNTAGI2C2K:
            return "NTAG I2C 2K";
        case MfUltralightTypeNTAGI2CPlus1K:
            return "NTAG I2C Plus 1K";
        case MfUltralightTypeNTAGI2CPlus2K:
            return "NTAG I2C Plus 2K";
        default:
            return "Mifare Ultralight";
    }
  }
}

const char* nfc_mf_classic_type(MfClassicType type) {
    switch (type) {
        case MfClassicType1k:
            return "Mifare Classic 1K";
        case MfClassicType4k:
            return "Mifare Classic 4K";
        default:
            return "Mifare Classic";
    }
}
