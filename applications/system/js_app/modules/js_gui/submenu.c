#include "../../js_modules.h" // IWYU pragma: keep
#include "js_gui.h"
#include "../js_event_loop/js_event_loop.h"
#include <gui/modules/submenu.h>

#define QUEUE_LEN 2

typedef struct {
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

static bool items_assign(struct mjs* mjs, Submenu* submenu, JsViewPropValue value, void* context) {
    UNUSED(mjs);
    submenu_reset(submenu);
    size_t len = mjs_array_length(mjs, value.array);
    for(size_t i = 0; i < len; i++) {
        mjs_val_t item = mjs_array_get(mjs, value.array, i);
        if(!mjs_is_string(item)) return false;
        submenu_add_item(submenu, mjs_get_string(mjs, &item, NULL), i, choose_callback, context);
    }
    return true;
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
    .prop_cnt = 2,
    .props = {
        (JsViewPropDescriptor){
            .name = "header",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)header_assign},
        (JsViewPropDescriptor){
            .name = "items",
            .type = JsViewPropTypeArr,
            .assign = (JsViewPropAssign)items_assign},
    }};
JS_GUI_VIEW_DEF(submenu, &view_descriptor);
