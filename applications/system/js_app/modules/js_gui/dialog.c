#include "../../js_modules.h" // IWYU pragma: keep
#include "js_gui.h"
#include "../js_event_loop/js_event_loop.h"
#include <gui/modules/dialog_ex.h>

#define QUEUE_LEN 2

typedef struct {
    FuriMessageQueue* queue;
    JsEventLoopContract contract;
} JsDialogCtx;

static mjs_val_t
    input_transformer(struct mjs* mjs, FuriMessageQueue* queue, JsDialogCtx* context) {
    UNUSED(context);
    DialogExResult result;
    furi_check(furi_message_queue_get(queue, &result, 0) == FuriStatusOk);
    const char* string;
    if(result == DialogExResultLeft) {
        string = "left";
    } else if(result == DialogExResultCenter) {
        string = "center";
    } else if(result == DialogExResultRight) {
        string = "right";
    } else {
        furi_crash();
    }
    return mjs_mk_string(mjs, string, ~0, false);
}

static void input_callback(DialogExResult result, JsDialogCtx* context) {
    furi_check(furi_message_queue_put(context->queue, &result, 0) == FuriStatusOk);
}

static bool
    header_assign(struct mjs* mjs, DialogEx* dialog, JsViewPropValue value, JsDialogCtx* context) {
    UNUSED(mjs);
    UNUSED(context);
    dialog_ex_set_header(dialog, value.string, 64, 0, AlignCenter, AlignTop);
    return true;
}

static bool
    text_assign(struct mjs* mjs, DialogEx* dialog, JsViewPropValue value, JsDialogCtx* context) {
    UNUSED(mjs);
    UNUSED(context);
    dialog_ex_set_text(dialog, value.string, 64, 32, AlignCenter, AlignCenter);
    return true;
}

static bool
    left_assign(struct mjs* mjs, DialogEx* dialog, JsViewPropValue value, JsDialogCtx* context) {
    UNUSED(mjs);
    UNUSED(context);
    dialog_ex_set_left_button_text(dialog, value.string);
    return true;
}
static bool
    center_assign(struct mjs* mjs, DialogEx* dialog, JsViewPropValue value, JsDialogCtx* context) {
    UNUSED(mjs);
    UNUSED(context);
    dialog_ex_set_center_button_text(dialog, value.string);
    return true;
}
static bool
    right_assign(struct mjs* mjs, DialogEx* dialog, JsViewPropValue value, JsDialogCtx* context) {
    UNUSED(mjs);
    UNUSED(context);
    dialog_ex_set_right_button_text(dialog, value.string);
    return true;
}

static JsDialogCtx* ctx_make(struct mjs* mjs, DialogEx* dialog, mjs_val_t view_obj) {
    JsDialogCtx* context = malloc(sizeof(JsDialogCtx));
    context->queue = furi_message_queue_alloc(QUEUE_LEN, sizeof(DialogExResult));
    context->contract = (JsEventLoopContract){
        .magic = JsForeignMagic_JsEventLoopContract,
        .object_type = JsEventLoopObjectTypeQueue,
        .object = context->queue,
        .non_timer =
            {
                .event = FuriEventLoopEventIn,
                .transformer = (JsEventLoopTransformer)input_transformer,
            },
    };
    mjs_set(mjs, view_obj, "input", ~0, mjs_mk_foreign(mjs, &context->contract));
    dialog_ex_set_result_callback(dialog, (DialogExResultCallback)input_callback);
    dialog_ex_set_context(dialog, context);
    return context;
}

static void ctx_destroy(DialogEx* input, JsDialogCtx* context, FuriEventLoop* loop) {
    UNUSED(input);
    furi_event_loop_maybe_unsubscribe(loop, context->queue);
    furi_message_queue_free(context->queue);
    free(context);
}

static const JsViewDescriptor view_descriptor = {
    .alloc = (JsViewAlloc)dialog_ex_alloc,
    .free = (JsViewFree)dialog_ex_free,
    .get_view = (JsViewGetView)dialog_ex_get_view,
    .custom_make = (JsViewCustomMake)ctx_make,
    .custom_destroy = (JsViewCustomDestroy)ctx_destroy,
    .prop_cnt = 5,
    .props = {
        (JsViewPropDescriptor){
            .name = "header",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)header_assign},
        (JsViewPropDescriptor){
            .name = "text",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)text_assign},
        (JsViewPropDescriptor){
            .name = "left",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)left_assign},
        (JsViewPropDescriptor){
            .name = "center",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)center_assign},
        (JsViewPropDescriptor){
            .name = "right",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)right_assign},
    }};

JS_GUI_VIEW_DEF(dialog, &view_descriptor);
