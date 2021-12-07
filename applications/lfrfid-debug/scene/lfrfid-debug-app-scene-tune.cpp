#include "lfrfid-debug-app-scene-tune.h"
#include <furi-hal.h>

extern COMP_HandleTypeDef hcomp1;

static void comparator_trigger_callback(void* hcomp, void* comp_ctx) {
    COMP_HandleTypeDef* _hcomp = static_cast<COMP_HandleTypeDef*>(hcomp);

    if(hcomp == &hcomp1) {
        hal_gpio_write(&gpio_ext_pa7, HAL_COMP_GetOutputLevel(_hcomp) == COMP_OUTPUT_LEVEL_HIGH);
    }
}

void LfRfidDebugAppSceneTune::on_enter(LfRfidDebugApp* app, bool need_restore) {
    app->view_controller.switch_to<LfRfidViewTuneVM>();
    hal_gpio_init_simple(&gpio_ext_pa7, GpioModeOutputPushPull);

    api_interrupt_add(comparator_trigger_callback, InterruptTypeComparatorTrigger, this);

    hcomp1.Init.InputMinus = COMP_INPUT_MINUS_1_2VREFINT;
    hcomp1.Init.InputPlus = COMP_INPUT_PLUS_IO1;
    hcomp1.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
    hcomp1.Init.Hysteresis = COMP_HYSTERESIS_HIGH;
    hcomp1.Init.BlankingSrce = COMP_BLANKINGSRC_NONE;
    hcomp1.Init.Mode = COMP_POWERMODE_MEDIUMSPEED;
    hcomp1.Init.WindowMode = COMP_WINDOWMODE_DISABLE;
    hcomp1.Init.TriggerMode = COMP_TRIGGERMODE_IT_RISING_FALLING;
    if(HAL_COMP_Init(&hcomp1) != HAL_OK) {
        Error_Handler();
    }

    HAL_COMP_Start(&hcomp1);

    furi_hal_rfid_pins_read();
    furi_hal_rfid_tim_read(125000, 0.5);
    furi_hal_rfid_tim_read_start();
}

bool LfRfidDebugAppSceneTune::on_event(LfRfidDebugApp* app, LfRfidDebugApp::Event* event) {
    bool consumed = false;

    LfRfidViewTuneVM* tune = app->view_controller;

    if(tune->is_dirty()) {
        furi_hal_rfid_set_read_period(tune->get_ARR());
        furi_hal_rfid_set_read_pulse(tune->get_CCR());
    }

    return consumed;
}

void LfRfidDebugAppSceneTune::on_exit(LfRfidDebugApp* app) {
    HAL_COMP_Stop(&hcomp1);
    api_interrupt_remove(comparator_trigger_callback, InterruptTypeComparatorTrigger);

    hal_gpio_init_simple(&gpio_ext_pa7, GpioModeAnalog);
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_tim_reset();
    furi_hal_rfid_pins_reset();
}