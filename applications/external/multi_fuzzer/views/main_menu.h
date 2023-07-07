#pragma once

#include <gui/view.h>
#include "../helpers/fuzzer_custom_event.h"
#include "../helpers/fuzzer_types.h"

typedef struct FuzzerViewMain FuzzerViewMain;

typedef void (*FuzzerViewMainCallback)(FuzzerCustomEvent event, void* context);

void fuzzer_view_main_set_callback(
    FuzzerViewMain* fuzzer_view_main,
    FuzzerViewMainCallback callback,
    void* context);

FuzzerViewMain* fuzzer_view_main_alloc();

void fuzzer_view_main_free(FuzzerViewMain* view);

View* fuzzer_view_main_get_view(FuzzerViewMain* view);

void fuzzer_view_main_update_data(FuzzerViewMain* view, FuzzerState state);
void fuzzer_view_main_get_state(FuzzerViewMain* view, FuzzerState* state);