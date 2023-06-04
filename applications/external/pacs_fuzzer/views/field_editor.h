#pragma once

#include <gui/view.h>
#include "../helpers/fuzzer_custom_event.h"

typedef struct FuzzerViewFieldEditor FuzzerViewFieldEditor;

typedef void (*FuzzerViewFieldEditorCallback)(FuzzerCustomEvent event, void* context);

void fuzzer_view_field_editor_set_callback(
    FuzzerViewFieldEditor* view_attack,
    FuzzerViewFieldEditorCallback callback,
    void* context);

FuzzerViewFieldEditor* fuzzer_view_field_editor_alloc();

void fuzzer_view_field_editor_free(FuzzerViewFieldEditor* view_attack);

View* fuzzer_view_field_editor_get_view(FuzzerViewFieldEditor* view_attack);

void fuzzer_view_field_editor_reset_data(
    FuzzerViewFieldEditor* view_edit,
    uint8_t* uid,
    uint8_t uid_size);

const uint8_t* fuzzer_view_field_editor_get_uid(FuzzerViewFieldEditor* view_edit);

uint8_t fuzzer_view_field_editor_get_index(FuzzerViewFieldEditor* view_edit);