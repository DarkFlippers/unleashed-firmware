#include "dolphin_deed.h"
#include <furi.h>

static const DolphinDeedWeight dolphin_deed_weights[] = {
    {1, DolphinAppSubGhz}, // DolphinDeedSubGhzReceiverInfo
    {3, DolphinAppSubGhz}, // DolphinDeedSubGhzSave
    {1, DolphinAppSubGhz}, // DolphinDeedSubGhzRawRec
    {2, DolphinAppSubGhz}, // DolphinDeedSubGhzAddManually
    {2, DolphinAppSubGhz}, // DolphinDeedSubGhzSend
    {1, DolphinAppSubGhz}, // DolphinDeedSubGhzFrequencyAnalyzer

    {1, DolphinAppRfid}, // DolphinDeedRfidRead
    {3, DolphinAppRfid}, // DolphinDeedRfidReadSuccess
    {3, DolphinAppRfid}, // DolphinDeedRfidSave
    {2, DolphinAppRfid}, // DolphinDeedRfidEmulate
    {2, DolphinAppRfid}, // DolphinDeedRfidAdd

    {1, DolphinAppNfc}, // DolphinDeedNfcRead
    {3, DolphinAppNfc}, // DolphinDeedNfcReadSuccess
    {3, DolphinAppNfc}, // DolphinDeedNfcSave
    {1, DolphinAppNfc}, // DolphinDeedNfcDetectReader
    {2, DolphinAppNfc}, // DolphinDeedNfcEmulate
    {2, DolphinAppNfc}, // DolphinDeedNfcMfcAdd
    {1, DolphinAppNfc}, // DolphinDeedNfcAddSave
    {1, DolphinAppNfc}, // DolphinDeedNfcAddEmulate

    {1, DolphinAppIr}, // DolphinDeedIrSend
    {3, DolphinAppIr}, // DolphinDeedIrLearnSuccess
    {3, DolphinAppIr}, // DolphinDeedIrSave

    {1, DolphinAppIbutton}, // DolphinDeedIbuttonRead
    {3, DolphinAppIbutton}, // DolphinDeedIbuttonReadSuccess
    {3, DolphinAppIbutton}, // DolphinDeedIbuttonSave
    {2, DolphinAppIbutton}, // DolphinDeedIbuttonEmulate
    {2, DolphinAppIbutton}, // DolphinDeedIbuttonAdd

    {3, DolphinAppBadusb}, // DolphinDeedBadUsbPlayScript
    {3, DolphinAppPlugin}, // DolphinDeedU2fAuthorized

    {1, DolphinAppPlugin}, // DolphinDeedGpioUartBridge

    {2, DolphinAppPlugin}, // DolphinDeedPluginStart
    {1, DolphinAppPlugin}, // DolphinDeedPluginGameStart
    {10, DolphinAppPlugin}, // DolphinDeedPluginGameWin
};

static uint8_t dolphin_deed_limits[] = {
    20, // DolphinAppSubGhz
    20, // DolphinAppRfid
    20, // DolphinAppNfc
    20, // DolphinAppIr
    20, // DolphinAppIbutton
    20, // DolphinAppBadusb
    20, // DolphinAppPlugin
};

_Static_assert(COUNT_OF(dolphin_deed_weights) == DolphinDeedMAX, "dolphin_deed_weights size error");
_Static_assert(COUNT_OF(dolphin_deed_limits) == DolphinAppMAX, "dolphin_deed_limits size error");

uint8_t dolphin_deed_get_weight(DolphinDeed deed) {
    furi_check(deed < DolphinDeedMAX);
    return dolphin_deed_weights[deed].icounter;
}

DolphinApp dolphin_deed_get_app(DolphinDeed deed) {
    furi_check(deed < DolphinDeedMAX);
    return dolphin_deed_weights[deed].app;
}

uint8_t dolphin_deed_get_app_limit(DolphinApp app) {
    furi_check(app < DolphinAppMAX);
    return dolphin_deed_limits[app];
}
