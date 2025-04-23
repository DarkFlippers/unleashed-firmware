#include "../../js_modules.h" // IWYU pragma: keep
#include "./js_gui.h"
#include <furi.h>
#include <mlib/m-array.h>
#include <gui/view_dispatcher.h>
#include "../js_event_loop/js_event_loop.h"
#include <m-array.h>

#define EVENT_QUEUE_SIZE 16

typedef struct {
    uint32_t next_view_id;
    FuriEventLoop* loop;
    Gui* gui;
    ViewDispatcher* dispatcher;
    // event stuff
    JsEventLoopContract custom_contract;
    FuriMessageQueue* custom;
    JsEventLoopContract navigation_contract;
    FuriSemaphore*
        navigation; // FIXME: (-nofl) convert into callback once FuriEventLoop starts supporting this
} JsGui;

// Useful for factories
static JsGui* js_gui;

typedef struct {
    uint32_t id;
    const JsViewDescriptor* descriptor;
    void* specific_view;
    void* custom_data;
} JsGuiViewData;

/**
 * @brief Transformer for custom events
 */
static mjs_val_t
    js_gui_vd_custom_transformer(struct mjs* mjs, FuriEventLoopObject* object, void* context) {
    UNUSED(context);
    furi_check(object);
    FuriMessageQueue* queue = object;
    uint32_t event;
    furi_check(furi_message_queue_get(queue, &event, 0) == FuriStatusOk);
    return mjs_mk_number(mjs, (double)event);
}

/**
 * @brief ViewDispatcher custom event callback
 */
static bool js_gui_vd_custom_callback(void* context, uint32_t event) {
    furi_check(context);
    JsGui* module = context;
    furi_check(furi_message_queue_put(module->custom, &event, 0) == FuriStatusOk);
    return true;
}

/**
 * @brief ViewDispatcher navigation event callback
 */
static bool js_gui_vd_nav_callback(void* context) {
    furi_check(context);
    JsGui* module = context;
    furi_semaphore_release(module->navigation);
    return true;
}

/**
 * @brief `viewDispatcher.sendCustom`
 */
static void js_gui_vd_send_custom(struct mjs* mjs) {
    static const JsValueDeclaration js_gui_vd_send_custom_arg_list[] = {
        JS_VALUE_SIMPLE(JsValueTypeInt32),
    };
    static const JsValueArguments js_gui_vd_send_custom_args =
        JS_VALUE_ARGS(js_gui_vd_send_custom_arg_list);

    int32_t event;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_gui_vd_send_custom_args, &event);

    JsGui* module = JS_GET_CONTEXT(mjs);
    view_dispatcher_send_custom_event(module->dispatcher, (uint32_t)event);
}

/**
 * @brief `viewDispatcher.sendTo`
 */
static void js_gui_vd_send_to(struct mjs* mjs) {
    typedef enum {
        JsSendDirToFront,
        JsSendDirToBack,
    } JsSendDir;
    static const JsValueEnumVariant js_send_dir_variants[] = {
        {"front", JsSendDirToFront},
        {"back", JsSendDirToBack},
    };
    static const JsValueDeclaration js_gui_vd_send_to_arg_list[] = {
        JS_VALUE_ENUM(JsSendDir, js_send_dir_variants),
    };
    static const JsValueArguments js_gui_vd_send_to_args =
        JS_VALUE_ARGS(js_gui_vd_send_to_arg_list);

    JsSendDir send_direction;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_gui_vd_send_to_args, &send_direction);

    JsGui* module = JS_GET_CONTEXT(mjs);
    if(send_direction == JsSendDirToBack) {
        view_dispatcher_send_to_back(module->dispatcher);
    } else {
        view_dispatcher_send_to_front(module->dispatcher);
    }
}

/**
 * @brief `viewDispatcher.switchTo`
 */
static void js_gui_vd_switch_to(struct mjs* mjs) {
    static const JsValueDeclaration js_gui_vd_switch_to_arg_list[] = {
        JS_VALUE_SIMPLE(JsValueTypeAny),
    };
    static const JsValueArguments js_gui_vd_switch_to_args =
        JS_VALUE_ARGS(js_gui_vd_switch_to_arg_list);

    mjs_val_t view;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_gui_vd_switch_to_args, &view);

    JsGuiViewData* view_data = JS_GET_INST(mjs, view);
    mjs_val_t vd_obj = mjs_get_this(mjs);
    JsGui* module = JS_GET_INST(mjs, vd_obj);
    view_dispatcher_switch_to_view(module->dispatcher, (uint32_t)view_data->id);
    mjs_set(mjs, vd_obj, "currentView", ~0, view);
}

static void* js_gui_create(struct mjs* mjs, mjs_val_t* object, JsModules* modules) {
    // get event loop
    JsEventLoop* js_loop = js_module_get(modules, "event_loop");
    if(M_UNLIKELY(!js_loop)) return NULL;
    FuriEventLoop* loop = js_event_loop_get_loop(js_loop);

    // create C object
    JsGui* module = malloc(sizeof(JsGui));
    module->loop = loop;
    module->gui = furi_record_open(RECORD_GUI);
    module->dispatcher = view_dispatcher_alloc_ex(loop);
    module->custom = furi_message_queue_alloc(EVENT_QUEUE_SIZE, sizeof(uint32_t));
    module->navigation = furi_semaphore_alloc(EVENT_QUEUE_SIZE, 0);
    view_dispatcher_attach_to_gui(module->dispatcher, module->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_send_to_front(module->dispatcher);

    // subscribe to events and create contracts
    view_dispatcher_set_event_callback_context(module->dispatcher, module);
    view_dispatcher_set_custom_event_callback(module->dispatcher, js_gui_vd_custom_callback);
    view_dispatcher_set_navigation_event_callback(module->dispatcher, js_gui_vd_nav_callback);
    module->custom_contract = (JsEventLoopContract){
        .magic = JsForeignMagic_JsEventLoopContract,
        .object = module->custom,
        .object_type = JsEventLoopObjectTypeQueue,
        .non_timer =
            {
                .event = FuriEventLoopEventIn,
                .transformer = js_gui_vd_custom_transformer,
            },
    };
    module->navigation_contract = (JsEventLoopContract){
        .magic = JsForeignMagic_JsEventLoopContract,
        .object = module->navigation,
        .object_type = JsEventLoopObjectTypeSemaphore,
        .non_timer =
            {
                .event = FuriEventLoopEventIn,
            },
    };

    // create viewDispatcher object
    mjs_val_t view_dispatcher = mjs_mk_object(mjs);
    JS_ASSIGN_MULTI(mjs, view_dispatcher) {
        JS_FIELD(INST_PROP_NAME, mjs_mk_foreign(mjs, module));
        JS_FIELD("sendCustom", MJS_MK_FN(js_gui_vd_send_custom));
        JS_FIELD("sendTo", MJS_MK_FN(js_gui_vd_send_to));
        JS_FIELD("switchTo", MJS_MK_FN(js_gui_vd_switch_to));
        JS_FIELD("custom", mjs_mk_foreign(mjs, &module->custom_contract));
        JS_FIELD("navigation", mjs_mk_foreign(mjs, &module->navigation_contract));
        JS_FIELD("currentView", MJS_NULL);
    }

    // create API object
    mjs_val_t api = mjs_mk_object(mjs);
    mjs_set(mjs, api, "viewDispatcher", ~0, view_dispatcher);

    *object = api;
    js_gui = module;
    return module;
}

static void js_gui_destroy(void* inst) {
    furi_assert(inst);
    JsGui* module = inst;

    view_dispatcher_free(module->dispatcher);
    furi_event_loop_maybe_unsubscribe(module->loop, module->custom);
    furi_event_loop_maybe_unsubscribe(module->loop, module->navigation);
    furi_message_queue_free(module->custom);
    furi_semaphore_free(module->navigation);

    furi_record_close(RECORD_GUI);
    free(module);
    js_gui = NULL;
}

/**
 * @brief Assigns a `View` property. Not available from JS.
 */
static bool
    js_gui_view_assign(struct mjs* mjs, const char* name, mjs_val_t value, JsGuiViewData* data) {
    const JsViewDescriptor* descriptor = data->descriptor;
    for(size_t i = 0; i < descriptor->prop_cnt; i++) {
        JsViewPropDescriptor prop = descriptor->props[i];
        if(strcmp(prop.name, name) != 0) continue;

        // convert JS value to C
        JsViewPropValue c_value;
        const char* expected_type = NULL;
        switch(prop.type) {
        case JsViewPropTypeNumber: {
            if(!mjs_is_number(value)) {
                expected_type = "number";
                break;
            }
            c_value = (JsViewPropValue){.number = mjs_get_int32(mjs, value)};
        } break;
        case JsViewPropTypeString: {
            if(!mjs_is_string(value)) {
                expected_type = "string";
                break;
            }
            c_value = (JsViewPropValue){.string = mjs_get_string(mjs, &value, NULL)};
        } break;
        case JsViewPropTypeArr: {
            if(!mjs_is_array(value)) {
                expected_type = "array";
                break;
            }
            c_value = (JsViewPropValue){.term = value};
        } break;
        case JsViewPropTypeTypedArr: {
            if(!mjs_is_typed_array(value)) {
                expected_type = "typed_array";
                break;
            }
            c_value = (JsViewPropValue){.term = value};
        } break;
        case JsViewPropTypeBool: {
            if(!mjs_is_boolean(value)) {
                expected_type = "bool";
                break;
            }
            c_value = (JsViewPropValue){.boolean = mjs_get_bool(mjs, value)};
        } break;
        }

        if(expected_type) {
            mjs_prepend_errorf(
                mjs, MJS_BAD_ARGS_ERROR, "view prop \"%s\" requires %s value", name, expected_type);
            return false;
        } else {
            return prop.assign(mjs, data->specific_view, c_value, data->custom_data);
        }
    }

    mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "view has no prop named \"%s\"", name);
    return false;
}

/**
 * @brief Sets the list of children. Not available from JS.
 */
static bool
    js_gui_view_internal_set_children(struct mjs* mjs, mjs_val_t children, JsGuiViewData* data) {
    data->descriptor->reset_children(data->specific_view, data->custom_data);

    for(size_t i = 0; i < mjs_array_length(mjs, children); i++) {
        mjs_val_t child = mjs_array_get(mjs, children, i);
        if(!data->descriptor->add_child(mjs, data->specific_view, data->custom_data, child))
            return false;
    }

    return true;
}

/**
 * @brief `View.set`
 */
static void js_gui_view_set(struct mjs* mjs) {
    static const JsValueDeclaration js_gui_view_set_arg_list[] = {
        JS_VALUE_SIMPLE(JsValueTypeString),
        JS_VALUE_SIMPLE(JsValueTypeAny),
    };
    static const JsValueArguments js_gui_view_set_args = JS_VALUE_ARGS(js_gui_view_set_arg_list);

    const char* name;
    mjs_val_t value;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_gui_view_set_args, &name, &value);

    JsGuiViewData* data = JS_GET_CONTEXT(mjs);
    bool success = js_gui_view_assign(mjs, name, value, data);
    UNUSED(success);
    mjs_return(mjs, MJS_UNDEFINED);
}

/**
 * @brief `View.addChild`
 */
static void js_gui_view_add_child(struct mjs* mjs) {
    static const JsValueDeclaration js_gui_view_add_child_arg_list[] = {
        JS_VALUE_SIMPLE(JsValueTypeAny),
    };
    static const JsValueArguments js_gui_view_add_child_args =
        JS_VALUE_ARGS(js_gui_view_add_child_arg_list);

    mjs_val_t child;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_gui_view_add_child_args, &child);

    JsGuiViewData* data = JS_GET_CONTEXT(mjs);
    if(!data->descriptor->add_child || !data->descriptor->reset_children)
        JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "this View can't have children");

    bool success = data->descriptor->add_child(mjs, data->specific_view, data->custom_data, child);
    UNUSED(success);
    mjs_return(mjs, MJS_UNDEFINED);
}

/**
 * @brief `View.resetChildren`
 */
static void js_gui_view_reset_children(struct mjs* mjs) {
    JsGuiViewData* data = JS_GET_CONTEXT(mjs);
    if(!data->descriptor->add_child || !data->descriptor->reset_children)
        JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "this View can't have children");

    data->descriptor->reset_children(data->specific_view, data->custom_data);
    mjs_return(mjs, MJS_UNDEFINED);
}

/**
 * @brief `View.setChildren`
 */
static void js_gui_view_set_children(struct mjs* mjs) {
    static const JsValueDeclaration js_gui_view_set_children_arg_list[] = {
        JS_VALUE_SIMPLE(JsValueTypeAnyArray),
    };
    static const JsValueArguments js_gui_view_set_children_args =
        JS_VALUE_ARGS(js_gui_view_set_children_arg_list);

    mjs_val_t children;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_gui_view_set_children_args, &children);

    JsGuiViewData* data = JS_GET_CONTEXT(mjs);
    if(!data->descriptor->add_child || !data->descriptor->reset_children)
        JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "this View can't have children");

    js_gui_view_internal_set_children(mjs, children, data);
}

/**
 * @brief `View` destructor
 */
static void js_gui_view_destructor(struct mjs* mjs, mjs_val_t obj) {
    JsGuiViewData* data = JS_GET_INST(mjs, obj);
    view_dispatcher_remove_view(js_gui->dispatcher, data->id);
    if(data->descriptor->custom_destroy)
        data->descriptor->custom_destroy(data->specific_view, data->custom_data, js_gui->loop);
    data->descriptor->free(data->specific_view);
    free(data);
}

/**
 * @brief Creates a `View` object from a descriptor. Not available from JS.
 */
static mjs_val_t js_gui_make_view(struct mjs* mjs, const JsViewDescriptor* descriptor) {
    void* specific_view = descriptor->alloc();
    View* view = descriptor->get_view(specific_view);
    uint32_t view_id = js_gui->next_view_id++;
    view_dispatcher_add_view(js_gui->dispatcher, view_id, view);

    // generic view API
    mjs_val_t view_obj = mjs_mk_object(mjs);
    JS_ASSIGN_MULTI(mjs, view_obj) {
        JS_FIELD("set", MJS_MK_FN(js_gui_view_set));
        JS_FIELD("addChild", MJS_MK_FN(js_gui_view_add_child));
        JS_FIELD("resetChildren", MJS_MK_FN(js_gui_view_reset_children));
        JS_FIELD("setChildren", MJS_MK_FN(js_gui_view_set_children));
    }

    // object data
    JsGuiViewData* data = malloc(sizeof(JsGuiViewData));
    *data = (JsGuiViewData){
        .descriptor = descriptor,
        .id = view_id,
        .specific_view = specific_view,
        .custom_data =
            descriptor->custom_make ? descriptor->custom_make(mjs, specific_view, view_obj) : NULL,
    };
    mjs_set(mjs, view_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, data));
    mjs_set(mjs, view_obj, MJS_DESTRUCTOR_PROP_NAME, ~0, MJS_MK_FN(js_gui_view_destructor));

    return view_obj;
}

/**
 * @brief `ViewFactory.make`
 */
static void js_gui_vf_make(struct mjs* mjs) {
    const JsViewDescriptor* descriptor = JS_GET_CONTEXT(mjs);
    mjs_return(mjs, js_gui_make_view(mjs, descriptor));
}

/**
 * @brief `ViewFactory.makeWith`
 */
static void js_gui_vf_make_with(struct mjs* mjs) {
    static const JsValueDeclaration js_gui_vf_make_with_arg_list[] = {
        JS_VALUE_SIMPLE(JsValueTypeAnyObject),
        JS_VALUE_SIMPLE(JsValueTypeAny),
    };
    static const JsValueArguments js_gui_vf_make_with_args =
        JS_VALUE_ARGS(js_gui_vf_make_with_arg_list);

    mjs_val_t props, children;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_gui_vf_make_with_args, &props, &children);
    const JsViewDescriptor* descriptor = JS_GET_CONTEXT(mjs);

    // make the object like normal
    mjs_val_t view_obj = js_gui_make_view(mjs, descriptor);
    JsGuiViewData* data = JS_GET_INST(mjs, view_obj);

    // assign properties one by one
    mjs_val_t key, iter = MJS_UNDEFINED;
    while((key = mjs_next(mjs, props, &iter)) != MJS_UNDEFINED) {
        furi_check(mjs_is_string(key));
        const char* name = mjs_get_string(mjs, &key, NULL);
        mjs_val_t value = mjs_get(mjs, props, name, ~0);

        if(!js_gui_view_assign(mjs, name, value, data)) {
            mjs_return(mjs, MJS_UNDEFINED);
            return;
        }
    }

    // assign children
    if(mjs_is_array(children)) {
        if(!data->descriptor->add_child || !data->descriptor->reset_children)
            JS_ERROR_AND_RETURN(mjs, MJS_BAD_ARGS_ERROR, "this View can't have children");

        if(!js_gui_view_internal_set_children(mjs, children, data)) return;
    }

    mjs_return(mjs, view_obj);
}

mjs_val_t js_gui_make_view_factory(struct mjs* mjs, const JsViewDescriptor* view_descriptor) {
    mjs_val_t factory = mjs_mk_object(mjs);
    mjs_set(mjs, factory, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, (void*)view_descriptor));
    mjs_set(mjs, factory, "make", ~0, MJS_MK_FN(js_gui_vf_make));
    mjs_set(mjs, factory, "makeWith", ~0, MJS_MK_FN(js_gui_vf_make_with));
    return factory;
}

extern const ElfApiInterface js_gui_hashtable_api_interface;

static const JsModuleDescriptor js_gui_desc = {
    "gui",
    js_gui_create,
    js_gui_destroy,
    &js_gui_hashtable_api_interface,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_gui_desc,
};

const FlipperAppPluginDescriptor* js_gui_ep(void) {
    return &plugin_descriptor;
}
