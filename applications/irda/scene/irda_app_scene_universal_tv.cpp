#include <stdint.h>
#include <gui/modules/loading.h>
#include <gui/view_stack.h>
#include "irda/scene/irda_app_scene.h"
#include "irda/irda_app.h"

void IrdaAppSceneUniversalTV::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    ButtonPanel* button_panel = view_manager->get_button_panel();
    button_panel_reserve(button_panel, 2, 3);

    int i = 0;
    button_panel_add_item(
        button_panel,
        i,
        0,
        0,
        3,
        19,
        &I_Power_25x27,
        &I_Power_hvr_25x27,
        irda_app_item_callback,
        app);
    brute_force.add_record(i, "POWER");
    ++i;
    button_panel_add_item(
        button_panel,
        i,
        1,
        0,
        36,
        19,
        &I_Mute_25x27,
        &I_Mute_hvr_25x27,
        irda_app_item_callback,
        app);
    brute_force.add_record(i, "MUTE");
    ++i;
    button_panel_add_item(
        button_panel,
        i,
        0,
        1,
        3,
        66,
        &I_Vol_up_25x27,
        &I_Vol_up_hvr_25x27,
        irda_app_item_callback,
        app);
    brute_force.add_record(i, "VOL+");
    ++i;
    button_panel_add_item(
        button_panel, i, 1, 1, 36, 66, &I_Up_25x27, &I_Up_hvr_25x27, irda_app_item_callback, app);
    brute_force.add_record(i, "CH+");
    ++i;
    button_panel_add_item(
        button_panel,
        i,
        0,
        2,
        3,
        98,
        &I_Vol_down_25x27,
        &I_Vol_down_hvr_25x27,
        irda_app_item_callback,
        app);
    brute_force.add_record(i, "VOL-");
    ++i;
    button_panel_add_item(
        button_panel,
        i,
        1,
        2,
        36,
        98,
        &I_Down_25x27,
        &I_Down_hvr_25x27,
        irda_app_item_callback,
        app);
    brute_force.add_record(i, "CH-");

    button_panel_add_label(button_panel, 6, 11, FontPrimary, "TV remote");
    button_panel_add_label(button_panel, 9, 64, FontSecondary, "Vol");
    button_panel_add_label(button_panel, 43, 64, FontSecondary, "Ch");

    view_manager->switch_to(IrdaAppViewManager::ViewType::UniversalRemote);

    auto stack_view = app->get_view_manager()->get_universal_view_stack();
    auto loading_view = app->get_view_manager()->get_loading();
    view_stack_add_view(stack_view, loading_get_view(loading_view));

    /**
     * Problem: Update events are not handled in Loading View, because:
     * 1) Timer task has least prio
     * 2) Storage service uses drivers that capture whole CPU time
     *      to handle SD communication
     *
     * Ugly workaround, but it works for current situation:
     * raise timer task prio for DB scanning period.
     */
    TaskHandle_t timer_task = xTaskGetHandle(configTIMER_SERVICE_TASK_NAME);
    TaskHandle_t storage_task = xTaskGetHandle("StorageSrv");
    uint32_t timer_prio = uxTaskPriorityGet(timer_task);
    uint32_t storage_prio = uxTaskPriorityGet(storage_task);
    vTaskPrioritySet(timer_task, storage_prio + 1);
    bool result = brute_force.calculate_messages();
    vTaskPrioritySet(timer_task, timer_prio);

    view_stack_remove_view(stack_view, loading_get_view(loading_view));

    if(!result) {
        app->switch_to_previous_scene();
    }
}
