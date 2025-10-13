#include "../../js_modules.h" // IWYU pragma: keep
#include "js_gui.h"
#include "../js_event_loop/js_event_loop.h"
#include <gui/modules/variable_item_list.h>
#include <toolbox/str_buffer.h>

typedef struct {
    StrBuffer str_buffer;

    // let mjs do the memory management heavy lifting, store children in a js array
    struct mjs* mjs;
    mjs_val_t children;
    VariableItemList* list;

    FuriMessageQueue* input_queue;
    JsEventLoopContract contract;
} JsViListContext;

typedef struct {
    int32_t item_index;
    int32_t value_index;
} JsViListEvent;

static mjs_val_t
    input_transformer(struct mjs* mjs, FuriMessageQueue* queue, JsViListContext* context) {
    UNUSED(context);
    JsViListEvent event;
    furi_check(furi_message_queue_get(queue, &event, 0) == FuriStatusOk);

    mjs_val_t event_obj = mjs_mk_object(mjs);
    JS_ASSIGN_MULTI(mjs, event_obj) {
        JS_FIELD("itemIndex", mjs_mk_number(mjs, event.item_index));
        JS_FIELD("valueIndex", mjs_mk_number(mjs, event.value_index));
    }

    return event_obj;
}

static void js_vi_list_change_callback(VariableItem* item) {
    JsViListContext* context = variable_item_get_context(item);
    struct mjs* mjs = context->mjs;
    uint8_t item_index = variable_item_list_get_selected_item_index(context->list);
    uint8_t value_index = variable_item_get_current_value_index(item);

    // type safety ensured in add_child
    mjs_val_t variants = mjs_array_get(mjs, context->children, item_index);
    mjs_val_t variant = mjs_array_get(mjs, variants, value_index);
    variable_item_set_current_value_text(item, mjs_get_string(mjs, &variant, NULL));

    JsViListEvent event = {
        .item_index = item_index,
        .value_index = value_index,
    };
    furi_check(furi_message_queue_put(context->input_queue, &event, 0) == FuriStatusOk);
}

static bool js_vi_list_add_child(
    struct mjs* mjs,
    VariableItemList* list,
    JsViListContext* context,
    mjs_val_t child_obj) {
    static const JsValueDeclaration js_vi_list_string = JS_VALUE_SIMPLE(JsValueTypeString);
    static const JsValueDeclaration js_vi_list_arr = JS_VALUE_SIMPLE(JsValueTypeAnyArray);
    static const JsValueDeclaration js_vi_list_int_default_0 =
        JS_VALUE_SIMPLE_W_DEFAULT(JsValueTypeInt32, int32_val, 0);

    static const JsValueObjectField js_vi_list_child_fields[] = {
        {"label", &js_vi_list_string},
        {"variants", &js_vi_list_arr},
        {"defaultSelected", &js_vi_list_int_default_0},
    };
    static const JsValueDeclaration js_vi_list_child =
        JS_VALUE_OBJECT_W_DEFAULTS(js_vi_list_child_fields);

    JsValueParseStatus status;
    const char* label;
    mjs_val_t variants;
    int32_t default_selected;
    JS_VALUE_PARSE(
        mjs,
        JS_VALUE_PARSE_SOURCE_VALUE(&js_vi_list_child),
        JsValueParseFlagReturnOnError,
        &status,
        &child_obj,
        &label,
        &variants,
        &default_selected);
    if(status != JsValueParseStatusOk) return false;

    size_t variants_cnt = mjs_array_length(mjs, variants);
    for(size_t i = 0; i < variants_cnt; i++)
        if(!mjs_is_string(mjs_array_get(mjs, variants, i))) return false;

    VariableItem* item = variable_item_list_add(
        list,
        str_buffer_make_owned_clone(&context->str_buffer, label),
        variants_cnt,
        js_vi_list_change_callback,
        context);
    variable_item_set_current_value_index(item, default_selected);
    mjs_val_t default_variant = mjs_array_get(mjs, variants, default_selected);
    variable_item_set_current_value_text(item, mjs_get_string(mjs, &default_variant, NULL));

    mjs_array_push(context->mjs, context->children, variants);

    return true;
}

static void js_vi_list_reset_children(VariableItemList* list, JsViListContext* context) {
    mjs_disown(context->mjs, &context->children);
    context->children = mjs_mk_array(context->mjs);
    mjs_own(context->mjs, &context->children);

    variable_item_list_reset(list);
    str_buffer_clear_all_clones(&context->str_buffer);
}

static JsViListContext* ctx_make(struct mjs* mjs, VariableItemList* list, mjs_val_t view_obj) {
    JsViListContext* context = malloc(sizeof(JsViListContext));
    *context = (JsViListContext){
        .str_buffer = {0},
        .mjs = mjs,
        .children = mjs_mk_array(mjs),
        .list = list,
        .input_queue = furi_message_queue_alloc(1, sizeof(JsViListEvent)),
    };
    mjs_own(context->mjs, &context->children);
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
    mjs_set(mjs, view_obj, "valueUpdate", ~0, mjs_mk_foreign(mjs, &context->contract));
    return context;
}

static void ctx_destroy(VariableItemList* input, JsViListContext* context, FuriEventLoop* loop) {
    UNUSED(input);
    furi_event_loop_maybe_unsubscribe(loop, context->input_queue);
    furi_message_queue_free(context->input_queue);
    str_buffer_clear_all_clones(&context->str_buffer);
    free(context);
}

static const JsViewDescriptor view_descriptor = {
    .alloc = (JsViewAlloc)variable_item_list_alloc,
    .free = (JsViewFree)variable_item_list_free,
    .get_view = (JsViewGetView)variable_item_list_get_view,
    .custom_make = (JsViewCustomMake)ctx_make,
    .custom_destroy = (JsViewCustomDestroy)ctx_destroy,
    .add_child = (JsViewAddChild)js_vi_list_add_child,
    .reset_children = (JsViewResetChildren)js_vi_list_reset_children,
    .prop_cnt = 0,
    .props = {},
};

JS_GUI_VIEW_DEF(vi_list, &view_descriptor);
