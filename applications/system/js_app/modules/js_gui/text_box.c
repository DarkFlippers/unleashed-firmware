#include "../../js_modules.h" // IWYU pragma: keep
#include "js_gui.h"
#include <gui/modules/text_box.h>

static bool
    text_assign(struct mjs* mjs, TextBox* text_box, JsViewPropValue value, FuriString* context) {
    UNUSED(mjs);
    furi_string_set(context, value.string);
    text_box_set_text(text_box, furi_string_get_cstr(context));
    return true;
}

static bool font_assign(struct mjs* mjs, TextBox* text_box, JsViewPropValue value, void* context) {
    UNUSED(context);
    TextBoxFont font;
    if(strcasecmp(value.string, "hex") == 0) {
        font = TextBoxFontHex;
    } else if(strcasecmp(value.string, "text") == 0) {
        font = TextBoxFontText;
    } else {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "must be one of: \"text\", \"hex\"");
        return false;
    }
    text_box_set_font(text_box, font);
    return true;
}

static bool
    focus_assign(struct mjs* mjs, TextBox* text_box, JsViewPropValue value, void* context) {
    UNUSED(context);
    TextBoxFocus focus;
    if(strcasecmp(value.string, "start") == 0) {
        focus = TextBoxFocusStart;
    } else if(strcasecmp(value.string, "end") == 0) {
        focus = TextBoxFocusEnd;
    } else {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "must be one of: \"start\", \"end\"");
        return false;
    }
    text_box_set_focus(text_box, focus);
    return true;
}

FuriString* ctx_make(struct mjs* mjs, TextBox* specific_view, mjs_val_t view_obj) {
    UNUSED(mjs);
    UNUSED(specific_view);
    UNUSED(view_obj);
    return furi_string_alloc();
}

void ctx_destroy(TextBox* specific_view, FuriString* context, FuriEventLoop* loop) {
    UNUSED(specific_view);
    UNUSED(loop);
    furi_string_free(context);
}

static const JsViewDescriptor view_descriptor = {
    .alloc = (JsViewAlloc)text_box_alloc,
    .free = (JsViewFree)text_box_free,
    .get_view = (JsViewGetView)text_box_get_view,
    .custom_make = (JsViewCustomMake)ctx_make,
    .custom_destroy = (JsViewCustomDestroy)ctx_destroy,
    .prop_cnt = 3,
    .props = {
        (JsViewPropDescriptor){
            .name = "text",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)text_assign},
        (JsViewPropDescriptor){
            .name = "font",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)font_assign},
        (JsViewPropDescriptor){
            .name = "focus",
            .type = JsViewPropTypeString,
            .assign = (JsViewPropAssign)focus_assign},
    }};
JS_GUI_VIEW_DEF(text_box, &view_descriptor);
