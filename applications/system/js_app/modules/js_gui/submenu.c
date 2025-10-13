#include "../../js_modules.h" // IWYU pragma: keep
#include "js_gui.h"
#include "../js_event_loop/js_event_loop.h"
#include <gui/modules/submenu.h>

#define QUEUE_LEN 2

typedef struct {
    int32_t next_index;
    FuriMessageQueue* queue;
    JsEventLoopContract contract;
} JsSubmenuCtx;

static mjs_val_t choose_transformer(struct mjs* mjs, FuriMessageQueue* queue, void* context) {
    UNUSED(context);
    uint32_t index;
    furi_check(furi_message_queue_get(queue, &index, 0) == FuriStatusOk);
    return mjs_mk_number(mjs, (double)index);
}

void choose_callback(void* context, uint32_t index) {
    JsSubmenuCtx* ctx = context;
    furi_check(furi_message_queue_put(ctx->queue, &index, 0) == FuriStatusOk);
}

static bool
    header_assign(struct mjs* mjs, Submenu* submenu, JsViewPropValue value, void* context) {
    UNUSED(mjs);
    UNUSED(context);
    submenu_set_header(submenu, value.string);
    return true;
}

static bool js_submenu_add_child(
    struct mjs* mjs,
    Submenu* submenu,
    JsSubmenuCtx* context,
    mjs_val_t child_obj) {
    const char* str = mjs_get_string(mjs, &child_obj, NULL);
    if(!str) return false;

    submenu_add_item(submenu, str, context->next_index++, choose_callback, context);

    return true;
}

static void js_submenu_reset_children(Submenu* submenu, JsSubmenuCtx* context) {
    context->next_index = 0;
    submenu_reset(submenu);
}

static JsSubmenuCtx* ctx_make(struct mjs* mjs, Submenu* input, mjs_val_t view_obj) {
    UNUSED(input);
    JsSubmenuCtx* context = malloc(sizeof(JsSubmenuCtx));
    context->queue = furi_message_queue_alloc(QUEUE_LEN, sizeof(uint32_t));
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

static void ctx_destroy(Submenu* input, JsSubmenuCtx* context, FuriEventLoop* loop) {
    UNUSED(input);
    furi_event_loop_maybe_unsubscribe(loop, context->queue);
    furi_message_queue_free(context->queue);
    free(context);
}

static const JsViewDescriptor view_descriptor = {
    .alloc = (JsViewAlloc)submenu_alloc,
    .free = (JsViewFree)submenu_free,
    .get_view = (JsViewGetView)submenu_get_view,
    .custom_make = (JsViewCustomMake)ctx_make,
    .custom_destroy = (JsViewCustomDestroy)ctx_destroy,
    .add_child = (JsViewAddChild)js_submenu_add_child,
    .reset_children = (JsViewResetChildren)js_submenu_reset_children,
    .prop_cnt = 1,
    .props = {
        (JsViewPropDescriptor){
            .name = "header",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)header_assign},
    }};
JS_GUI_VIEW_DEF(submenu, &view_descriptor);
