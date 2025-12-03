#pragma once

#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/elements.h>
#include <gui/scene_manager.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/date_time_input.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <input/input.h>

#include "scenes/example_date_time_input_scene.h"

typedef struct ExampleDateTimeInputShowDateTime ExampleDateTimeInputShowDateTime;

typedef enum {
    ExampleDateTimeInputViewIdShowDateTime,
    ExampleDateTimeInputViewIdDateTimeInput,
} ExampleDateTimeInputViewId;

typedef struct {
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    DateTimeInput* date_time_input;
    DialogEx* dialog_ex;

    DateTime date_time;

    bool edit_date;
    bool edit_time;
} ExampleDateTimeInput;
