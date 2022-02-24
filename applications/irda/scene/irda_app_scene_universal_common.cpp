#include <dolphin/dolphin.h>
#include <gui/modules/button_menu.h>
#include <gui/modules/button_panel.h>
#include <gui/view.h>
#include <gui/view_stack.h>

#include "../irda_app.h"
#include "irda/irda_app_event.h"
#include "irda/irda_app_view_manager.h"
#include "irda/scene/irda_app_scene.h"
#include "../view/irda_progress_view.h"

void IrdaAppSceneUniversalCommon::irda_app_item_callback(void* context, uint32_t index) {
    IrdaApp* app = static_cast<IrdaApp*>(context);
    IrdaAppEvent event;

    event.type = IrdaAppEvent::Type::ButtonPanelPressed;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

static void irda_progress_back_callback(void* context) {
    furi_assert(context);
    auto app = static_cast<IrdaApp*>(context);

    IrdaAppEvent irda_event = {
        .type = IrdaAppEvent::Type::Back,
    };
    app->get_view_manager()->clear_events();
    app->get_view_manager()->send_event(&irda_event);
}

void IrdaAppSceneUniversalCommon::remove_popup(IrdaApp* app) {
    auto stack_view = app->get_view_manager()->get_universal_view_stack();
    auto progress_view = app->get_view_manager()->get_progress();
    view_stack_remove_view(stack_view, irda_progress_view_get_view(progress_view));
}

void IrdaAppSceneUniversalCommon::show_popup(IrdaApp* app, int record_amount) {
    auto stack_view = app->get_view_manager()->get_universal_view_stack();
    auto progress_view = app->get_view_manager()->get_progress();
    irda_progress_view_set_progress_total(progress_view, record_amount);
    irda_progress_view_set_back_callback(progress_view, irda_progress_back_callback, app);
    view_stack_add_view(stack_view, irda_progress_view_get_view(progress_view));
}

bool IrdaAppSceneUniversalCommon::progress_popup(IrdaApp* app) {
    auto progress_view = app->get_view_manager()->get_progress();
    return irda_progress_view_increase_progress(progress_view);
}

bool IrdaAppSceneUniversalCommon::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(brute_force_started) {
        if(event->type == IrdaAppEvent::Type::Tick) {
            auto view_manager = app->get_view_manager();
            IrdaAppEvent tick_event = {.type = IrdaAppEvent::Type::Tick};
            view_manager->send_event(&tick_event);
            bool result = brute_force.send_next_bruteforce();
            if(result) {
                result = progress_popup(app);
            }
            if(!result) {
                brute_force.stop_bruteforce();
                brute_force_started = false;
                remove_popup(app);
            }
            consumed = true;
        } else if(event->type == IrdaAppEvent::Type::Back) {
            brute_force_started = false;
            brute_force.stop_bruteforce();
            remove_popup(app);
            consumed = true;
        }
    } else {
        if(event->type == IrdaAppEvent::Type::ButtonPanelPressed) {
            int record_amount = 0;
            if(brute_force.start_bruteforce(event->payload.menu_index, record_amount)) {
                DOLPHIN_DEED(DolphinDeedIrBruteForce);
                brute_force_started = true;
                show_popup(app, record_amount);
            } else {
                app->switch_to_previous_scene();
            }
            consumed = true;
        } else if(event->type == IrdaAppEvent::Type::Back) {
            app->switch_to_previous_scene();
            consumed = true;
        }
    }

    return consumed;
}

void IrdaAppSceneUniversalCommon::on_exit(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    ButtonPanel* button_panel = view_manager->get_button_panel();
    button_panel_reset(button_panel);
}
