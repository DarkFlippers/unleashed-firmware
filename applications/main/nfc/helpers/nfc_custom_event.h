#pragma once

enum NfcCustomEvent {
    // Reserve first 100 events for button types and indexes, starting from 0
    NfcCustomEventReserved = 100,

    NfcCustomEventViewExit,
    NfcCustomEventWorkerExit,
    NfcCustomEventByteInputDone,
    NfcCustomEventTextInputDone,
    NfcCustomEventDictAttackDone,
    NfcCustomEventDictAttackSkip,
    NfcCustomEventRpcLoad,
    NfcCustomEventRpcSessionClose,
};
