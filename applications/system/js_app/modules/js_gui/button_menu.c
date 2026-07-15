#include "../../js_modules.h" // IWYU pragma: keep
#include "js_gui.h"
#include "../js_event_loop/js_event_loop.h"
#include <gui/modules/button_menu.h>
#include <toolbox/str_buffer.h>

typedef struct {
    int32_t next_index;
    StrBuffer str_buffer;

    FuriMessageQueue* input_queue;
    JsEventLoopContract contract;
} JsBtnMenuContext;

typedef struct {
    int32_t index;
    InputType input_type;
} JsBtnMenuEvent;

static const char* js_input_type_to_str(InputType type) {
    switch(type) {
    case InputTypePress:
        return "press";
    case InputTypeRelease:
        return "release";
    case InputTypeShort:
        return "short";
    case InputTypeLong:
        return "long";
    case InputTypeRepeat:
        return "repeat";
    default:
        furi_crash();
    }
}

static mjs_val_t
    input_transformer(struct mjs* mjs, FuriMessageQueue* queue, JsBtnMenuContext* context) {
    UNUSED(context);
    JsBtnMenuEvent event;
    furi_check(furi_message_queue_get(queue, &event, 0) == FuriStatusOk);

    mjs_val_t event_obj = mjs_mk_object(mjs);
    JS_ASSIGN_MULTI(mjs, event_obj) {
        JS_FIELD("index", mjs_mk_number(mjs, event.index));
        JS_FIELD("type", mjs_mk_string(mjs, js_input_type_to_str(event.input_type), ~0, false));
    }

    return event_obj;
}

static void input_callback(void* ctx, int32_t index, InputType type) {
    JsBtnMenuContext* context = ctx;
    JsBtnMenuEvent event = {
        .index = index,
        .input_type = type,
    };
    furi_check(furi_message_queue_put(context->input_queue, &event, 0) == FuriStatusOk);
}

static bool matrix_header_assign(
    struct mjs* mjs,
    ButtonMenu* menu,
    JsViewPropValue value,
    JsBtnMenuContext* context) {
    UNUSED(mjs);
    button_menu_set_header(menu, str_buffer_make_owned_clone(&context->str_buffer, value.string));
    return true;
}

static bool js_button_menu_add_child(
    struct mjs* mjs,
    ButtonMenu* menu,
    JsBtnMenuContext* context,
    mjs_val_t child_obj) {
    static const JsValueEnumVariant js_button_menu_item_type_variants[] = {
        {"common", ButtonMenuItemTypeCommon},
        {"control", ButtonMenuItemTypeControl},
    };
    static const JsValueDeclaration js_button_menu_item_type =
        JS_VALUE_ENUM(ButtonMenuItemType, js_button_menu_item_type_variants);

    static const JsValueDeclaration js_button_menu_string = JS_VALUE_SIMPLE(JsValueTypeString);

    static const JsValueObjectField js_button_menu_child_fields[] = {
        {"type", &js_button_menu_item_type},
        {"label", &js_button_menu_string},
    };
    static const JsValueDeclaration js_button_menu_child =
        JS_VALUE_OBJECT(js_button_menu_child_fields);

    ButtonMenuItemType item_type;
    const char* label;
    JsValueParseStatus status;
    JS_VALUE_PARSE(
        mjs,
        JS_VALUE_PARSE_SOURCE_VALUE(&js_button_menu_child),
        JsValueParseFlagReturnOnError,
        &status,
        &child_obj,
        &item_type,
        &label);
    if(status != JsValueParseStatusOk) return false;

    button_menu_add_item(
        menu,
        str_buffer_make_owned_clone(&context->str_buffer, label),
        context->next_index++,
        input_callback,
        item_type,
        context);

    return true;
}

static void js_button_menu_reset_children(ButtonMenu* menu, JsBtnMenuContext* context) {
    context->next_index = 0;
    button_menu_reset(menu);
    str_buffer_clear_all_clones(&context->str_buffer);
}

static JsBtnMenuContext* ctx_make(struct mjs* mjs, ButtonMenu* menu, mjs_val_t view_obj) {
    UNUSED(menu);
    JsBtnMenuContext* context = malloc(sizeof(JsBtnMenuContext));
    *context = (JsBtnMenuContext){
        .next_index = 0,
        .str_buffer = {0},
        .input_queue = furi_message_queue_alloc(1, sizeof(JsBtnMenuEvent)),
    };
    context->contract = (JsEventLoopContract){
        .magic = JsForeignMagic_JsEventLoopContract,
        .object_type = JsEventLoopObjectTypeQueue,
        .object = context->input_queue,
        .non_timer =
            {
                .event = FuriEventLoopEventIn,
                .transformer = (JsEventLoopTransformer)input_transformer,
                .transformer_context = context,
            },
    };
    mjs_set(mjs, view_obj, "input", ~0, mjs_mk_foreign(mjs, &context->contract));
    return context;
}

static void ctx_destroy(ButtonMenu* input, JsBtnMenuContext* context, FuriEventLoop* loop) {
    UNUSED(input);
    furi_event_loop_maybe_unsubscribe(loop, context->input_queue);
    furi_message_queue_free(context->input_queue);
    str_buffer_clear_all_clones(&context->str_buffer);
    free(context);
}

static const JsViewDescriptor view_descriptor = {
    .alloc = (JsViewAlloc)button_menu_alloc,
    .free = (JsViewFree)button_menu_free,
    .get_view = (JsViewGetView)button_menu_get_view,
    .custom_make = (JsViewCustomMake)ctx_make,
    .custom_destroy = (JsViewCustomDestroy)ctx_destroy,
    .add_child = (JsViewAddChild)js_button_menu_add_child,
    .reset_children = (JsViewResetChildren)js_button_menu_reset_children,
    .prop_cnt = 1,
    .props = {
        (JsViewPropDescriptor){
            .name = "header",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)matrix_header_assign},
    }};

JS_GUI_VIEW_DEF(button_menu, &view_descriptor);
