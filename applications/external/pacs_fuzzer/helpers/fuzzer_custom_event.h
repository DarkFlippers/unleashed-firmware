#pragma once

typedef enum {

    // FuzzerCustomEvent
    FuzzerCustomEventViewMainBack = 100,
    FuzzerCustomEventViewMainOk,

    FuzzerCustomEventViewAttackBack,
    FuzzerCustomEventViewAttackOk,
    FuzzerCustomEventViewAttackTick,
    FuzzerCustomEventViewAttackEnd,

    FuzzerCustomEventViewFieldEditorBack,
    FuzzerCustomEventViewFieldEditorOk,
} FuzzerCustomEvent;