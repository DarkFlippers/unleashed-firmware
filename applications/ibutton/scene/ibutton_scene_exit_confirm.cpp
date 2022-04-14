#include "ibutton_scene_exit_confirm.h"
#include "../ibutton_app.h"

static void widget_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    if(type == InputTypeShort) {
        event.type = iButtonEvent::Type::EventTypeWidgetButtonResult;
        event.payload.widget_button_result = result;
        app->get_view_manager()->send_event(&event);
    }
}

void iButtonSceneExitConfirm::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Widget* widget = view_manager->get_widget();

    widget_add_button_element(widget, GuiButtonTypeLeft, "Exit", widget_callback, app);
    widget_add_button_element(widget, GuiButtonTypeRight, "Stay", widget_callback, app);
    widget_add_string_element(
        widget, 64, 19, AlignCenter, AlignBottom, FontPrimary, "Exit to iButton menu");
    widget_add_string_element(
        widget, 64, 29, AlignCenter, AlignBottom, FontSecondary, "All unsaved data will be lost");

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewWidget);
}

bool iButtonSceneExitConfirm::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeWidgetButtonResult) {
        if(event->payload.widget_button_result == GuiButtonTypeLeft) {
            app->search_and_switch_to_previous_scene({iButtonApp::Scene::SceneStart});
        } else if(event->payload.widget_button_result == GuiButtonTypeRight) {
            app->switch_to_previous_scene();
        }
        consumed = true;
    } else if(event->type == iButtonEvent::Type::EventTypeBack) {
        consumed = true;
    }

    return consumed;
}

void iButtonSceneExitConfirm::on_exit(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    Widget* widget = view_manager->get_widget();
    widget_reset(widget);
}
