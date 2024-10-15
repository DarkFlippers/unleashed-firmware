#include "../../js_modules.h" // IWYU pragma: keep
#include "js_gui.h"
#include "../js_event_loop/js_event_loop.h"
#include <gui/modules/text_input.h>

#define DEFAULT_BUF_SZ 33

typedef struct {
    char* buffer;
    size_t buffer_size;
    FuriString* header;
    FuriSemaphore* input_semaphore;
    JsEventLoopContract contract;
} JsKbdContext;

static mjs_val_t
    input_transformer(struct mjs* mjs, FuriSemaphore* semaphore, JsKbdContext* context) {
    furi_check(furi_semaphore_acquire(semaphore, 0) == FuriStatusOk);
    return mjs_mk_string(mjs, context->buffer, ~0, true);
}

static void input_callback(JsKbdContext* context) {
    furi_semaphore_release(context->input_semaphore);
}

static bool
    header_assign(struct mjs* mjs, TextInput* input, JsViewPropValue value, JsKbdContext* context) {
    UNUSED(mjs);
    furi_string_set(context->header, value.string);
    text_input_set_header_text(input, furi_string_get_cstr(context->header));
    return true;
}

static bool min_len_assign(
    struct mjs* mjs,
    TextInput* input,
    JsViewPropValue value,
    JsKbdContext* context) {
    UNUSED(mjs);
    UNUSED(context);
    text_input_set_minimum_length(input, (size_t)value.number);
    return true;
}

static bool max_len_assign(
    struct mjs* mjs,
    TextInput* input,
    JsViewPropValue value,
    JsKbdContext* context) {
    UNUSED(mjs);
    UNUSED(input);
    context->buffer_size = (size_t)(value.number + 1);
    context->buffer = realloc(context->buffer, context->buffer_size); //-V701
    return true;
}

static bool default_text_assign(
    struct mjs* mjs,
    TextInput* input,
    JsViewPropValue value,
    JsKbdContext* context) {
    UNUSED(mjs);
    UNUSED(input);

    if(value.string) {
        strlcpy(context->buffer, value.string, context->buffer_size);
    }
    return true;
}

static bool default_text_clear_assign(
    struct mjs* mjs,
    TextInput* input,
    JsViewPropValue value,
    JsKbdContext* context) {
    UNUSED(mjs);

    text_input_set_result_callback(
        input,
        (TextInputCallback)input_callback,
        context,
        context->buffer,
        context->buffer_size,
        value.boolean);
    return true;
}

static JsKbdContext* ctx_make(struct mjs* mjs, TextInput* input, mjs_val_t view_obj) {
    UNUSED(input);
    JsKbdContext* context = malloc(sizeof(JsKbdContext));
    *context = (JsKbdContext){
        .buffer_size = DEFAULT_BUF_SZ,
        .buffer = malloc(DEFAULT_BUF_SZ),
        .header = furi_string_alloc(),
        .input_semaphore = furi_semaphore_alloc(1, 0),
    };
    context->contract = (JsEventLoopContract){
        .magic = JsForeignMagic_JsEventLoopContract,
        .object_type = JsEventLoopObjectTypeSemaphore,
        .object = context->input_semaphore,
        .non_timer =
            {
                .event = FuriEventLoopEventIn,
                .transformer = (JsEventLoopTransformer)input_transformer,
                .transformer_context = context,
            },
    };
    UNUSED(mjs);
    UNUSED(view_obj);
    mjs_set(mjs, view_obj, "input", ~0, mjs_mk_foreign(mjs, &context->contract));
    return context;
}

static void ctx_destroy(TextInput* input, JsKbdContext* context, FuriEventLoop* loop) {
    UNUSED(input);
    furi_event_loop_maybe_unsubscribe(loop, context->input_semaphore);
    furi_semaphore_free(context->input_semaphore);
    furi_string_free(context->header);
    free(context->buffer);
    free(context);
}

static const JsViewDescriptor view_descriptor = {
    .alloc = (JsViewAlloc)text_input_alloc,
    .free = (JsViewFree)text_input_free,
    .get_view = (JsViewGetView)text_input_get_view,
    .custom_make = (JsViewCustomMake)ctx_make,
    .custom_destroy = (JsViewCustomDestroy)ctx_destroy,
    .prop_cnt = 5,
    .props = {
        (JsViewPropDescriptor){
            .name = "header",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)header_assign},
        (JsViewPropDescriptor){
            .name = "minLength",
            .type = JsViewPropTypeNumber,
            .assign = (JsViewPropAssign)min_len_assign},
        (JsViewPropDescriptor){
            .name = "maxLength",
            .type = JsViewPropTypeNumber,
            .assign = (JsViewPropAssign)max_len_assign},
        (JsViewPropDescriptor){
            .name = "defaultText",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)default_text_assign},
        (JsViewPropDescriptor){
            .name = "defaultTextClear",
            .type = JsViewPropTypeBool,
            .assign = (JsViewPropAssign)default_text_clear_assign},
    }};

JS_GUI_VIEW_DEF(text_input, &view_descriptor);
