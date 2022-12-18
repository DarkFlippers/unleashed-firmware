#pragma once

typedef enum {
    // Reserve first 100 events for button types and indexes, starting from 0
    SubBruteCustomEventTypeReserved = 100,

    SubBruteCustomEventTypeBackPressed,
    SubBruteCustomEventTypeIndexSelected,
    SubBruteCustomEventTypeTransmitStarted,
    SubBruteCustomEventTypeError,
    SubBruteCustomEventTypeTransmitFinished,
    SubBruteCustomEventTypeTransmitNotStarted,
    SubBruteCustomEventTypeTransmitCustom,
    SubBruteCustomEventTypeSaveFile,
    SubBruteCustomEventTypeUpdateView,
    SubBruteCustomEventTypeChangeStepUp,
    SubBruteCustomEventTypeChangeStepDown,
    SubBruteCustomEventTypeChangeStepUpMore,
    SubBruteCustomEventTypeChangeStepDownMore,

    SubBruteCustomEventTypeMenuSelected,
    SubBruteCustomEventTypeTextEditDone,
    SubBruteCustomEventTypePopupClosed,

    SubBruteCustomEventTypeLoadFile,
} SubBruteCustomEvent;