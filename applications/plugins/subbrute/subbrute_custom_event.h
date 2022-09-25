#pragma once

#include <stdint.h>
#include <stddef.h>

typedef enum {
    // Reserve first 100 events for button types and indexes, starting from 0
    SubBruteCustomEventTypeReserved = 100,

    SubBruteCustomEventTypeBackPressed,
    SubBruteCustomEventTypeTextEditResult,
    SubBruteCustomEventTypeTransmitStarted,
    SubBruteCustomEventTypeTransmitFinished,
    SubBruteCustomEventTypeTransmitNotStarted,
    SubBruteCustomEventTypeTransmitCustom,
    SubBruteCustomEventTypeSaveFile,
    SubBruteCustomEventTypeSaveSuccess,
    SubBruteCustomEventTypeChangeStepUp,
    SubBruteCustomEventTypeChangeStepDown,
    SubBruteCustomEventTypeChangeStepUpMore,
    SubBruteCustomEventTypeChangeStepDownMore,

    SubBruteCustomEventTypeMenuSelected,
    SubBruteCustomEventTypeTextEditDone,
    SubBruteCustomEventTypePopupClosed,

    SubBruteCustomEventTypeLoadFile,
} SubBruteCustomEvent;