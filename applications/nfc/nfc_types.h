#pragma once

#include <rfal_nfc.h>
#include <st_errno.h>

static inline const char* nfc_get_dev_type(rfalNfcDevType type) {
    if(type == RFAL_NFC_LISTEN_TYPE_NFCA) {
        return "NFC-A";
    } else if(type == RFAL_NFC_LISTEN_TYPE_NFCB) {
        return "NFC-B";
    } else if(type == RFAL_NFC_LISTEN_TYPE_NFCF) {
        return "NFC-F";
    } else if(type == RFAL_NFC_LISTEN_TYPE_NFCB) {
        return "NFC-B";
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

typedef enum {
    NfcDeviceTypeNfca,
    NfcDeviceTypeNfcb,
    NfcDeviceTypeNfcf,
    NfcDeviceTypeNfcv,
    NfcDeviceTypeNfcMifare,
    NfcDeviceTypeEMV,
} NfcDeviceType;

typedef struct {
    char name[32];
    uint8_t number[8];
} EMVCard;

typedef struct {
    NfcDeviceType type;
    union {
        rfalNfcaListenDevice nfca;
        rfalNfcbListenDevice nfcb;
        rfalNfcfListenDevice nfcf;
        rfalNfcvListenDevice nfcv;
        EMVCard emv_card;
    };
} NfcDevice;

typedef enum {
    // Init states
    NfcWorkerStateNone,
    NfcWorkerStateBroken,
    NfcWorkerStateReady,
    // Main worker states
    NfcWorkerStatePoll,
    NfcWorkerStateReadEMV,
    NfcWorkerStateEmulateEMV,
    NfcWorkerStateEmulate,
    NfcWorkerStateField,
    // Transition
    NfcWorkerStateStop,
} NfcWorkerState;

typedef enum {
    NfcMessageTypeDetect,
    NfcMessageTypeReadEMV,
    NfcMessageTypeEmulateEMV,
    NfcMessageTypeEmulate,
    NfcMessageTypeField,
    NfcMessageTypeStop,
    NfcMessageTypeExit,
    // From Worker
    NfcMessageTypeDeviceFound,
    NfcMessageTypeDeviceNotFound,
    NfcMessageTypeEMVFound,
    NfcMessageTypeEMVNotFound,
} NfcMessageType;

typedef struct {
    NfcMessageType type;
    union {
        NfcDevice device;
    };
} NfcMessage;
