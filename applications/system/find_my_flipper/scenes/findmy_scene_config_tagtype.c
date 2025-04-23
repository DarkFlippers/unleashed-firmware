#include "../findmy_i.h"

enum VarItemListIndex {
    VarItemListIndexApple,
    VarItemListIndexSamsung,
    VarItemListIndexTile,
};

void findmy_scene_config_tagtype_callback(void* context, uint32_t index) {
    furi_assert(context);
    FindMy* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void findmy_scene_config_tagtype_on_enter(void* context) {
    FindMy* app = context;
    VariableItemList* var_item_list = app->var_item_list;
    VariableItem* item;

    //variable_item_list_set_header(var_item_list, "Choose tag type");

    item = variable_item_list_add(var_item_list, "Apple AirTag", 0, NULL, NULL);

    item = variable_item_list_add(var_item_list, "Samsung SmartTag", 0, NULL, NULL);

    item = variable_item_list_add(var_item_list, "Tile SmartTag", 0, NULL, NULL);

    UNUSED(item);

    variable_item_list_set_enter_callback(
        var_item_list, findmy_scene_config_tagtype_callback, app);

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(app->scene_manager, FindMySceneConfigImport));
    view_dispatcher_switch_to_view(app->view_dispatcher, FindMyViewVarItemList);
}

bool findmy_scene_config_tagtype_on_event(void* context, SceneManagerEvent event) {
    FindMy* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, FindMySceneConfigTagtype, event.event);
        consumed = true;

        switch(event.event) {
        case VarItemListIndexApple:
            findmy_set_tag_type(app, FindMyTypeApple);
            break;
        case VarItemListIndexSamsung:
            findmy_set_tag_type(app, FindMyTypeSamsung);
            break;
        case VarItemListIndexTile:
            findmy_set_tag_type(app, FindMyTypeTile);
            break;
        default:
            break;
        }
        scene_manager_next_scene(app->scene_manager, FindMySceneConfigImport);
    }

    return consumed;
}

void findmy_scene_config_tagtype_on_exit(void* context) {
    FindMy* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    variable_item_list_reset(var_item_list);
}
