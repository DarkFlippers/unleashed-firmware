#pragma once

#include "scenes/signal_gen_scene.h"

#include "furi_hal_clock.h"
#include "furi_hal_pwm.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/submenu.h>
#include "views/signal_gen_pwm.h"

typedef struct SignalGenApp SignalGenApp;

struct SignalGenApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    VariableItemList* var_item_list;
    Submenu* submenu;
    SignalGenPwm* pwm_view;

    FuriHalClockMcoSourceId mco_src;
    FuriHalClockMcoDivisorId mco_div;

    FuriHalPwmOutputId pwm_ch_prev;
    FuriHalPwmOutputId pwm_ch;
    uint32_t pwm_freq;
    uint8_t pwm_duty;
};

typedef enum {
    SignalGenViewVarItemList,
    SignalGenViewSubmenu,
    SignalGenViewPwm,
} SignalGenAppView;

typedef enum {
    SignalGenMcoEventUpdate,
    SignalGenPwmEventUpdate,
    SignalGenPwmEventChannelChange,
} SignalGenCustomEvent;
