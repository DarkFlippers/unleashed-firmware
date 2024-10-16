#include "../../js_modules.h" // IWYU pragma: keep
#include "js_gui.h"
#include "../js_event_loop/js_event_loop.h"
#include <gui/modules/byte_input.h>

#define DEFAULT_BUF_SZ 4

typedef struct {
    uint8_t* buffer;
    size_t buffer_size;
    FuriString* header;
    FuriSemaphore* input_semaphore;
    JsEventLoopContract contract;
} JsByteKbContext;

static mjs_val_t
    input_transformer(struct mjs* mjs, FuriSemaphore* semaphore, JsByteKbContext* context) {
    furi_check(furi_semaphore_acquire(semaphore, 0) == FuriStatusOk);
    return mjs_mk_array_buf(mjs, (char*)context->buffer, context->buffer_size);
}

static void input_callback(JsByteKbContext* context) {
    furi_semaphore_release(context->input_semaphore);
}

static bool header_assign(
    struct mjs* mjs,
    ByteInput* input,
    JsViewPropValue value,
    JsByteKbContext* context) {
    UNUSED(mjs);
    furi_string_set(context->header, value.string);
    byte_input_set_header_text(input, furi_string_get_cstr(context->header));
    return true;
}

static bool
    len_assign(struct mjs* mjs, ByteInput* input, JsViewPropValue value, JsByteKbContext* context) {
    UNUSED(mjs);
    UNUSED(input);
    context->buffer_size = (size_t)(value.number);
    context->buffer = realloc(context->buffer, context->buffer_size); //-V701
    byte_input_set_result_callback(
        input,
        (ByteInputCallback)input_callback,
        NULL,
        context,
        context->buffer,
        context->buffer_size);
    return true;
}

static bool default_data_assign(
    struct mjs* mjs,
    ByteInput* input,
    JsViewPropValue value,
    JsByteKbContext* context) {
    UNUSED(mjs);

    mjs_val_t array_buf = value.array;
    if(mjs_is_data_view(array_buf)) {
        array_buf = mjs_dataview_get_buf(mjs, array_buf);
    }
    size_t default_data_len = 0;
    char* default_data = mjs_array_buf_get_ptr(mjs, array_buf, &default_data_len);
    memcpy(
        context->buffer,
        (uint8_t*)default_data,
        MIN((size_t)context->buffer_size, default_data_len));

    byte_input_set_result_callback(
        input,
        (ByteInputCallback)input_callback,
        NULL,
        context,
        context->buffer,
        context->buffer_size);
    return true;
}

static JsByteKbContext* ctx_make(struct mjs* mjs, ByteInput* input, mjs_val_t view_obj) {
    JsByteKbContext* context = malloc(sizeof(JsByteKbContext));
    *context = (JsByteKbContext){
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
    byte_input_set_result_callback(
        input,
        (ByteInputCallback)input_callback,
        NULL,
        context,
        context->buffer,
        context->buffer_size);
    mjs_set(mjs, view_obj, "input", ~0, mjs_mk_foreign(mjs, &context->contract));
    return context;
}

static void ctx_destroy(ByteInput* input, JsByteKbContext* context, FuriEventLoop* loop) {
    UNUSED(input);
    furi_event_loop_maybe_unsubscribe(loop, context->input_semaphore);
    furi_semaphore_free(context->input_semaphore);
    furi_string_free(context->header);
    free(context->buffer);
    free(context);
}

static const JsViewDescriptor view_descriptor = {
    .alloc = (JsViewAlloc)byte_input_alloc,
    .free = (JsViewFree)byte_input_free,
    .get_view = (JsViewGetView)byte_input_get_view,
    .custom_make = (JsViewCustomMake)ctx_make,
    .custom_destroy = (JsViewCustomDestroy)ctx_destroy,
    .prop_cnt = 3,
    .props = {
        (JsViewPropDescriptor){
            .name = "header",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)header_assign},
        (JsViewPropDescriptor){
            .name = "length",
            .type = JsViewPropTypeNumber,
            .assign = (JsViewPropAssign)len_assign},
        (JsViewPropDescriptor){
            .name = "defaultData",
            .type = JsViewPropTypeTypedArr,
            .assign = (JsViewPropAssign)default_data_assign},
    }};

JS_GUI_VIEW_DEF(byte_input, &view_descriptor);
