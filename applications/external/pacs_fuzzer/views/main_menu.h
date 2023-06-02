#pragma once

#include <gui/view.h>
#include "../helpers/fuzzer_custom_event.h"

typedef enum {
    FuzzerViewMainStateIdle,
    FuzzerViewMainStateLoading,
    FuzzerViewMainStateSending,
    FuzzerViewMainStateOFF,
} FuzzerViewMainState;

typedef struct FuzzerViewMain FuzzerViewMain;

typedef void (*FuzzerViewMainCallback)(FuzzerCustomEvent event, void* context);

void fuzzer_view_main_set_callback(
    FuzzerViewMain* fuzzer_view_main,
    FuzzerViewMainCallback callback,
    void* context);

FuzzerViewMain* fuzzer_view_main_alloc();

void fuzzer_view_main_free(FuzzerViewMain* fuzzer_view_main);

View* fuzzer_view_main_get_view(FuzzerViewMain* fuzzer_view_main);