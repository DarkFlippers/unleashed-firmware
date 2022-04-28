#include <dolphin/dolphin.h>
#include <gui/modules/button_menu.h>
#include <gui/modules/button_panel.h>
#include <gui/view.h>
#include <gui/view_stack.h>

#include "../infrared_app.h"
#include "infrared/infrared_app_event.h"
#include "infrared/infrared_app_view_manager.h"
#include "infrared/scene/infrared_app_scene.h"
#include "../view/infrared_progress_view.h"

void InfraredAppSceneUniversalCommon::infrared_app_item_callback(void* context, uint32_t index) {
    InfraredApp* app = static_cast<InfraredApp*>(context);
    InfraredAppEvent event;

    event.type = InfraredAppEvent::Type::ButtonPanelPressed;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

static void infrared_progress_back_callback(void* context) {
    furi_assert(context);
    auto app = static_cast<InfraredApp*>(context);

    InfraredAppEvent infrared_event = {
        .type = InfraredAppEvent::Type::Back,
    };
    app->get_view_manager()->clear_events();
    app->get_view_manager()->send_event(&infrared_event);
}

void InfraredAppSceneUniversalCommon::hide_popup(InfraredApp* app) {
    auto stack_view = app->get_view_manager()->get_universal_view_stack();
    auto progress_view = app->get_view_manager()->get_progress();
    view_stack_remove_view(stack_view, infrared_progress_view_get_view(progress_view));
}

void InfraredAppSceneUniversalCommon::show_popup(InfraredApp* app, int record_amount) {
    auto stack_view = app->get_view_manager()->get_universal_view_stack();
    auto progress_view = app->get_view_manager()->get_progress();
    infrared_progress_view_set_progress_total(progress_view, record_amount);
    infrared_progress_view_set_back_callback(progress_view, infrared_progress_back_callback, app);
    view_stack_add_view(stack_view, infrared_progress_view_get_view(progress_view));
}

bool InfraredAppSceneUniversalCommon::progress_popup(InfraredApp* app) {
    auto progress_view = app->get_view_manager()->get_progress();
    return infrared_progress_view_increase_progress(progress_view);
}

bool InfraredAppSceneUniversalCommon::on_event(InfraredApp* app, InfraredAppEvent* event) {
    bool consumed = false;

    if(brute_force_started) {
        if(event->type == InfraredAppEvent::Type::Tick) {
            auto view_manager = app->get_view_manager();
            app->notify_blink_send();
            InfraredAppEvent tick_event = {.type = InfraredAppEvent::Type::Tick};
            view_manager->send_event(&tick_event);
            bool result = brute_force.send_next_bruteforce();
            if(result) {
                result = progress_popup(app);
            }
            if(!result) {
                brute_force.stop_bruteforce();
                brute_force_started = false;
                hide_popup(app);
            }
            consumed = true;
        } else if(event->type == InfraredAppEvent::Type::Back) {
            brute_force_started = false;
            brute_force.stop_bruteforce();
            hide_popup(app);
            consumed = true;
        }
    } else {
        if(event->type == InfraredAppEvent::Type::ButtonPanelPressed) {
            int record_amount = 0;
            if(brute_force.start_bruteforce(event->payload.menu_index, record_amount)) {
                DOLPHIN_DEED(DolphinDeedIrBruteForce);
                brute_force_started = true;
                show_popup(app, record_amount);
                app->notify_blink_send();
            } else {
                app->switch_to_previous_scene();
            }
            consumed = true;
        } else if(event->type == InfraredAppEvent::Type::Back) {
            app->switch_to_previous_scene();
            consumed = true;
        }
    }

    return consumed;
}

void InfraredAppSceneUniversalCommon::on_exit(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    ButtonPanel* button_panel = view_manager->get_button_panel();
    button_panel_reset(button_panel);
}
