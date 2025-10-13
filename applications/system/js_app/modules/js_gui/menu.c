#include "../../js_modules.h" // IWYU pragma: keep
#include "js_gui.h"
#include "../js_event_loop/js_event_loop.h"
#include <gui/modules/menu.h>
#include <toolbox/str_buffer.h>

typedef struct {
    int32_t next_index;
    StrBuffer str_buffer;

    FuriMessageQueue* queue;
    JsEventLoopContract contract;
} JsMenuCtx;

static mjs_val_t choose_transformer(struct mjs* mjs, FuriMessageQueue* queue, void* context) {
    UNUSED(context);
    uint32_t index;
    furi_check(furi_message_queue_get(queue, &index, 0) == FuriStatusOk);
    return mjs_mk_number(mjs, (double)index);
}

static void choose_callback(void* context, uint32_t index) {
    JsMenuCtx* ctx = context;
    furi_check(furi_message_queue_put(ctx->queue, &index, 0) == FuriStatusOk);
}

static bool
    js_menu_add_child(struct mjs* mjs, Menu* menu, JsMenuCtx* context, mjs_val_t child_obj) {
    static const JsValueDeclaration js_menu_string = JS_VALUE_SIMPLE(JsValueTypeString);
    static const JsValueDeclaration js_menu_pointer = JS_VALUE_SIMPLE(JsValueTypeRawPointer);

    static const JsValueObjectField js_menu_child_fields[] = {
        {"icon", &js_menu_pointer},
        {"label", &js_menu_string},
    };
    static const JsValueDeclaration js_menu_child = JS_VALUE_OBJECT(js_menu_child_fields);

    const Icon* icon;
    const char* label;
    JsValueParseStatus status;
    JS_VALUE_PARSE(
        mjs,
        JS_VALUE_PARSE_SOURCE_VALUE(&js_menu_child),
        JsValueParseFlagReturnOnError,
        &status,
        &child_obj,
        &icon,
        &label);
    if(status != JsValueParseStatusOk) return false;

    menu_add_item(
        menu,
        str_buffer_make_owned_clone(&context->str_buffer, label),
        icon,
        context->next_index++,
        choose_callback,
        context);

    return true;
}

static void js_menu_reset_children(Menu* menu, JsMenuCtx* context) {
    context->next_index = 0;
    menu_reset(menu);
    str_buffer_clear_all_clones(&context->str_buffer);
}

static JsMenuCtx* ctx_make(struct mjs* mjs, Menu* input, mjs_val_t view_obj) {
    UNUSED(input);
    JsMenuCtx* context = malloc(sizeof(JsMenuCtx));
    context->queue = furi_message_queue_alloc(1, sizeof(uint32_t));
    context->contract = (JsEventLoopContract){
        .magic = JsForeignMagic_JsEventLoopContract,
        .object_type = JsEventLoopObjectTypeQueue,
        .object = context->queue,
        .non_timer =
            {
                .event = FuriEventLoopEventIn,
                .transformer = (JsEventLoopTransformer)choose_transformer,
            },
    };
    mjs_set(mjs, view_obj, "chosen", ~0, mjs_mk_foreign(mjs, &context->contract));
    return context;
}

static void ctx_destroy(Menu* input, JsMenuCtx* context, FuriEventLoop* loop) {
    UNUSED(input);
    furi_event_loop_maybe_unsubscribe(loop, context->queue);
    furi_message_queue_free(context->queue);
    str_buffer_clear_all_clones(&context->str_buffer);
    free(context);
}

static const JsViewDescriptor view_descriptor = {
    .alloc = (JsViewAlloc)menu_alloc,
    .free = (JsViewFree)menu_free,
    .get_view = (JsViewGetView)menu_get_view,
    .custom_make = (JsViewCustomMake)ctx_make,
    .custom_destroy = (JsViewCustomDestroy)ctx_destroy,
    .add_child = (JsViewAddChild)js_menu_add_child,
    .reset_children = (JsViewResetChildren)js_menu_reset_children,
    .prop_cnt = 0,
    .props = {},
};
JS_GUI_VIEW_DEF(menu, &view_descriptor);
