#include "../irda_app.h"
#include "assets_icons.h"
#include "gui/modules/button_menu.h"
#include "gui/modules/button_panel.h"
#include "../view/irda_app_brut_view.h"
#include "gui/view.h"
#include "irda/irda_app_event.h"
#include "irda/irda_app_view_manager.h"
#include "irda/scene/irda_app_scene.h"

void IrdaAppSceneUniversalCommon::irda_app_item_callback(void* context, uint32_t index) {
    IrdaApp* app = static_cast<IrdaApp*>(context);
    IrdaAppEvent event;

    event.type = IrdaAppEvent::Type::ButtonPanelPressed;
    event.payload.menu_index = index;

    app->get_view_manager()->send_event(&event);
}

static bool irda_popup_brut_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    furi_assert(event);
    auto app = static_cast<IrdaApp*>(context);
    bool consumed = false;

    if((event->type == InputTypeShort) && (event->key == InputKeyBack)) {
        consumed = true;
        IrdaAppEvent irda_event;

        irda_event.type = IrdaAppEvent::Type::Back;
        app->get_view_manager()->send_event(&irda_event);
    }

    return consumed;
}

void IrdaAppSceneUniversalCommon::remove_popup(IrdaApp* app) {
    auto button_panel = app->get_view_manager()->get_button_panel();
    button_panel_set_popup_draw_callback(button_panel, NULL, NULL);
    button_panel_set_popup_input_callback(button_panel, NULL, NULL);
}

void IrdaAppSceneUniversalCommon::show_popup(IrdaApp* app, int record_amount) {
    auto button_panel = app->get_view_manager()->get_button_panel();
    auto popup_brut = app->get_view_manager()->get_popup_brut();
    popup_brut_set_progress_max(popup_brut, record_amount);
    button_panel_set_popup_draw_callback(button_panel, popup_brut_draw_callback, popup_brut);
    button_panel_set_popup_input_callback(button_panel, irda_popup_brut_input_callback, app);
}

bool IrdaAppSceneUniversalCommon::progress_popup(IrdaApp* app) {
    bool result = popup_brut_increase_progress(app->get_view_manager()->get_popup_brut());
    auto button_panel = app->get_view_manager()->get_button_panel();
    with_view_model_cpp(button_panel_get_view(button_panel), void*, model, { return true; });
    return result;
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
    button_panel_clean(button_panel);
}
