#include "../../js_modules.h" // IWYU pragma: keep
#include "js_gui.h"
#include "../js_event_loop/js_event_loop.h"
#include <gui/modules/popup.h>
#include <toolbox/str_buffer.h>

typedef struct {
    StrBuffer str_buffer;
    FuriSemaphore* semaphore;
    JsEventLoopContract contract;
} JsPopupCtx;

static void timeout_callback(JsPopupCtx* context) {
    furi_check(furi_semaphore_release(context->semaphore) == FuriStatusOk);
}

static bool
    header_assign(struct mjs* mjs, Popup* popup, JsViewPropValue value, JsPopupCtx* context) {
    UNUSED(mjs);
    UNUSED(context);
    popup_set_header(
        popup,
        str_buffer_make_owned_clone(&context->str_buffer, value.string),
        64,
        0,
        AlignCenter,
        AlignTop);
    return true;
}

static bool
    text_assign(struct mjs* mjs, Popup* popup, JsViewPropValue value, JsPopupCtx* context) {
    UNUSED(mjs);
    UNUSED(context);
    popup_set_text(
        popup,
        str_buffer_make_owned_clone(&context->str_buffer, value.string),
        64,
        32,
        AlignCenter,
        AlignCenter);
    return true;
}

static bool
    timeout_assign(struct mjs* mjs, Popup* popup, JsViewPropValue value, JsPopupCtx* context) {
    UNUSED(mjs);
    UNUSED(context);
    popup_set_timeout(popup, value.number);
    popup_enable_timeout(popup);
    return true;
}

static JsPopupCtx* ctx_make(struct mjs* mjs, Popup* popup, mjs_val_t view_obj) {
    JsPopupCtx* context = malloc(sizeof(JsPopupCtx));
    context->semaphore = furi_semaphore_alloc(1, 0);
    context->contract = (JsEventLoopContract){
        .magic = JsForeignMagic_JsEventLoopContract,
        .object_type = JsEventLoopObjectTypeSemaphore,
        .object = context->semaphore,
        .non_timer =
            {
                .event = FuriEventLoopEventIn,
            },
    };
    mjs_set(mjs, view_obj, "timeout", ~0, mjs_mk_foreign(mjs, &context->contract));
    popup_set_callback(popup, (PopupCallback)timeout_callback);
    popup_set_context(popup, context);
    return context;
}

static void ctx_destroy(Popup* popup, JsPopupCtx* context, FuriEventLoop* loop) {
    UNUSED(popup);
    furi_event_loop_maybe_unsubscribe(loop, context->semaphore);
    furi_semaphore_free(context->semaphore);
    str_buffer_clear_all_clones(&context->str_buffer);
    free(context);
}

static const JsViewDescriptor view_descriptor = {
    .alloc = (JsViewAlloc)popup_alloc,
    .free = (JsViewFree)popup_free,
    .get_view = (JsViewGetView)popup_get_view,
    .custom_make = (JsViewCustomMake)ctx_make,
    .custom_destroy = (JsViewCustomDestroy)ctx_destroy,
    .prop_cnt = 3,
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
            .name = "timeout",
            .type = JsViewPropTypeNumber,
            .assign = (JsViewPropAssign)timeout_assign},
    }};

JS_GUI_VIEW_DEF(popup, &view_descriptor);
