#pragma once

typedef enum {

    // FuzzerCustomEvent
    FuzzerCustomEventViewMainBack = 100,
    FuzzerCustomEventViewMainOk,
    FuzzerCustomEventViewMainPopupErr,

    FuzzerCustomEventViewAttackBack,
    FuzzerCustomEventViewAttackOk,
    // FuzzerCustomEventViewAttackTick, // now not use
    FuzzerCustomEventViewAttackEnd,

    FuzzerCustomEventViewFieldEditorBack,
    FuzzerCustomEventViewFieldEditorOk,
} FuzzerCustomEvent;