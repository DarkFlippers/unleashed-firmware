#pragma once

#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/scene_manager.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/number_input.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <input/input.h>

#include "scenes/example_number_input_scene.h"

typedef struct ExampleNumberInputShowNumber ExampleNumberInputShowNumber;

typedef enum {
    ExampleNumberInputViewIdShowNumber,
    ExampleNumberInputViewIdNumberInput,
} ExampleNumberInputViewId;

typedef struct {
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    NumberInput* number_input;
    DialogEx* dialog_ex;

    int32_t current_number;
    int32_t min_value;
    int32_t max_value;
} ExampleNumberInput;
