#include "../../js_modules.h" // IWYU pragma: keep
#include "js_gui.h"
#include "../js_event_loop/js_event_loop.h"
#include <gui/modules/number_input.h>

typedef struct {
    int32_t default_val, min_val, max_val;
    FuriMessageQueue* input_queue;
    JsEventLoopContract contract;
} JsNumKbdContext;

static mjs_val_t
    input_transformer(struct mjs* mjs, FuriMessageQueue* queue, JsNumKbdContext* context) {
    UNUSED(context);
    int32_t number;
    furi_check(furi_message_queue_get(queue, &number, 0) == FuriStatusOk);
    return mjs_mk_number(mjs, number);
}

static void input_callback(void* ctx, int32_t value) {
    JsNumKbdContext* context = ctx;
    furi_check(furi_message_queue_put(context->input_queue, &value, 0) == FuriStatusOk);
}

static bool header_assign(
    struct mjs* mjs,
    NumberInput* input,
    JsViewPropValue value,
    JsNumKbdContext* context) {
    UNUSED(mjs);
    UNUSED(context);
    number_input_set_header_text(input, value.string);
    return true;
}

static bool min_val_assign(
    struct mjs* mjs,
    NumberInput* input,
    JsViewPropValue value,
    JsNumKbdContext* context) {
    UNUSED(mjs);
    context->min_val = value.number;
    number_input_set_result_callback(
        input, input_callback, context, context->default_val, context->min_val, context->max_val);
    return true;
}

static bool max_val_assign(
    struct mjs* mjs,
    NumberInput* input,
    JsViewPropValue value,
    JsNumKbdContext* context) {
    UNUSED(mjs);
    context->max_val = value.number;
    number_input_set_result_callback(
        input, input_callback, context, context->default_val, context->min_val, context->max_val);
    return true;
}

static bool default_val_assign(
    struct mjs* mjs,
    NumberInput* input,
    JsViewPropValue value,
    JsNumKbdContext* context) {
    UNUSED(mjs);
    context->default_val = value.number;
    number_input_set_result_callback(
        input, input_callback, context, context->default_val, context->min_val, context->max_val);
    return true;
}

static JsNumKbdContext* ctx_make(struct mjs* mjs, NumberInput* input, mjs_val_t view_obj) {
    JsNumKbdContext* context = malloc(sizeof(JsNumKbdContext));
    *context = (JsNumKbdContext){
        .default_val = 0,
        .max_val = 100,
        .min_val = 0,
        .input_queue = furi_message_queue_alloc(1, sizeof(int32_t)),
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
    number_input_set_result_callback(
        input, input_callback, context, context->default_val, context->min_val, context->max_val);
    mjs_set(mjs, view_obj, "input", ~0, mjs_mk_foreign(mjs, &context->contract));
    return context;
}

static void ctx_destroy(NumberInput* input, JsNumKbdContext* context, FuriEventLoop* loop) {
    UNUSED(input);
    furi_event_loop_maybe_unsubscribe(loop, context->input_queue);
    furi_message_queue_free(context->input_queue);
    free(context);
}

static const JsViewDescriptor view_descriptor = {
    .alloc = (JsViewAlloc)number_input_alloc,
    .free = (JsViewFree)number_input_free,
    .get_view = (JsViewGetView)number_input_get_view,
    .custom_make = (JsViewCustomMake)ctx_make,
    .custom_destroy = (JsViewCustomDestroy)ctx_destroy,
    .prop_cnt = 4,
    .props = {
        (JsViewPropDescriptor){
            .name = "header",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)header_assign},
        (JsViewPropDescriptor){
            .name = "minValue",
            .type = JsViewPropTypeNumber,
            .assign = (JsViewPropAssign)min_val_assign},
        (JsViewPropDescriptor){
            .name = "maxValue",
            .type = JsViewPropTypeNumber,
            .assign = (JsViewPropAssign)max_val_assign},
        (JsViewPropDescriptor){
            .name = "defaultValue",
            .type = JsViewPropTypeNumber,
            .assign = (JsViewPropAssign)default_val_assign},
    }};

JS_GUI_VIEW_DEF(number_input, &view_descriptor);
