#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DolphinAppSubGhz,
    DolphinAppRfid,
    DolphinAppNfc,
    DolphinAppIr,
    DolphinAppIbutton,
    DolphinAppBadusb,
    DolphinAppU2f,
    DolphinAppMAX,
} DolphinApp;

typedef enum {
    DolphinDeedSubGhzReceiverInfo,
    DolphinDeedSubGhzSave,
    DolphinDeedSubGhzRawRec,
    DolphinDeedSubGhzAddManually,
    DolphinDeedSubGhzSend,
    DolphinDeedSubGhzFrequencyAnalyzer,

    DolphinDeedRfidRead,
    DolphinDeedRfidReadSuccess,
    DolphinDeedRfidSave,
    DolphinDeedRfidEmulate,
    DolphinDeedRfidAdd,

    DolphinDeedNfcRead,
    DolphinDeedNfcReadSuccess,
    DolphinDeedNfcSave,
    DolphinDeedNfcEmulate,
    DolphinDeedNfcAdd,

    DolphinDeedIrSend,
    DolphinDeedIrLearnSuccess,
    DolphinDeedIrSave,
    DolphinDeedIrBruteForce,

    DolphinDeedIbuttonRead,
    DolphinDeedIbuttonReadSuccess,
    DolphinDeedIbuttonSave,
    DolphinDeedIbuttonEmulate,
    DolphinDeedIbuttonAdd,

    DolphinDeedBadUsbPlayScript,

    DolphinDeedU2fAuthorized,

    DolphinDeedMAX,

    DolphinDeedTestLeft,
    DolphinDeedTestRight,
} DolphinDeed;

typedef struct {
    uint8_t icounter;
    DolphinApp app;
} DolphinDeedWeight;

typedef struct {
    DolphinApp app;
    uint8_t icounter_limit;
} DolphinDeedLimits;

DolphinApp dolphin_deed_get_app(DolphinDeed deed);
uint8_t dolphin_deed_get_app_limit(DolphinApp app);
uint8_t dolphin_deed_get_weight(DolphinDeed deed);

#ifdef __cplusplus
}
#endif
