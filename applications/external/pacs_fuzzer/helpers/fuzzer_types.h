#pragma once

#include <furi.h>

typedef struct {
    uint8_t menu_index;
    uint8_t proto_index;
} FuzzerState;

typedef enum {
    FuzzerAttackStateOff = 0,
    FuzzerAttackStateIdle,
    FuzzerAttackStateRunning,
    FuzzerAttackStateEnd,

} FuzzerAttackState;

typedef enum {
    FuzzerFieldEditorStateEditingOn = 0,
    FuzzerFieldEditorStateEditingOff,

} FuzzerFieldEditorState;

typedef enum {
    FuzzerViewIDPopup,

    FuzzerViewIDMain,
    FuzzerViewIDAttack,
    FuzzerViewIDFieldEditor,
} FuzzerViewID;