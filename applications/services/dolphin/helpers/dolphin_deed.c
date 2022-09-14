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
    {2, DolphinAppNfc}, // DolphinDeedNfcEmulate
    {2, DolphinAppNfc}, // DolphinDeedNfcAdd

    {1, DolphinAppIr}, // DolphinDeedIrSend
    {3, DolphinAppIr}, // DolphinDeedIrLearnSuccess
    {3, DolphinAppIr}, // DolphinDeedIrSave
    {2, DolphinAppIr}, // DolphinDeedIrBruteForce

    {1, DolphinAppIbutton}, // DolphinDeedIbuttonRead
    {3, DolphinAppIbutton}, // DolphinDeedIbuttonReadSuccess
    {3, DolphinAppIbutton}, // DolphinDeedIbuttonSave
    {2, DolphinAppIbutton}, // DolphinDeedIbuttonEmulate
    {2, DolphinAppIbutton}, // DolphinDeedIbuttonAdd

    {3, DolphinAppBadusb}, // DolphinDeedBadUsbPlayScript
    {3, DolphinAppU2f}, // DolphinDeedU2fAuthorized
};

static uint8_t dolphin_deed_limits[] = {
    15, // DolphinAppSubGhz
    15, // DolphinAppRfid
    15, // DolphinAppNfc
    15, // DolphinAppIr
    15, // DolphinAppIbutton
    15, // DolphinAppBadusb
    15, // DolphinAppU2f
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
