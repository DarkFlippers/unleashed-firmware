#pragma once

#include <stdint.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>

#include "scenes/fuzzer_scene.h"
#include "views/main_menu.h"
#include "views/attack.h"

#include "helpers/fuzzer_types.h"

#include <flipper_format/flipper_format_i.h>

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    FuzzerViewMain* main_view;
    FuzzerViewAttack* attack_view;

    FuzzerState fuzzer_state;
} PacsFuzzerApp;