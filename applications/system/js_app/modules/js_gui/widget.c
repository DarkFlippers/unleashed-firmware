#include "../../js_modules.h" // IWYU pragma: keep
#include "js_gui.h"
#include "../js_event_loop/js_event_loop.h"
#include <gui/modules/widget.h>

typedef struct {
    FuriMessageQueue* queue;
    JsEventLoopContract contract;
} JsWidgetCtx;

#define QUEUE_LEN 2

/**
 * @brief Parses position (X and Y) from an element declaration object
 */
static bool element_get_position(struct mjs* mjs, mjs_val_t element, int32_t* x, int32_t* y) {
    mjs_val_t x_in = mjs_get(mjs, element, "x", ~0);
    mjs_val_t y_in = mjs_get(mjs, element, "y", ~0);
    if(!mjs_is_number(x_in) || !mjs_is_number(y_in)) return false;
    *x = mjs_get_int32(mjs, x_in);
    *y = mjs_get_int32(mjs, y_in);
    return true;
}

/**
 * @brief Parses size (W and h) from an element declaration object
 */
static bool element_get_size(struct mjs* mjs, mjs_val_t element, int32_t* w, int32_t* h) {
    mjs_val_t w_in = mjs_get(mjs, element, "w", ~0);
    mjs_val_t h_in = mjs_get(mjs, element, "h", ~0);
    if(!mjs_is_number(w_in) || !mjs_is_number(h_in)) return false;
    *w = mjs_get_int32(mjs, w_in);
    *h = mjs_get_int32(mjs, h_in);
    return true;
}

/**
 * @brief Parses alignment (V and H) from an element declaration object
 */
static bool
    element_get_alignment(struct mjs* mjs, mjs_val_t element, Align* align_v, Align* align_h) {
    mjs_val_t align_in = mjs_get(mjs, element, "align", ~0);
    const char* align = mjs_get_string(mjs, &align_in, NULL);
    if(!align) return false;
    if(strlen(align) != 2) return false;

    if(align[0] == 't') {
        *align_v = AlignTop;
    } else if(align[0] == 'c') {
        *align_v = AlignCenter;
    } else if(align[0] == 'b') {
        *align_v = AlignBottom;
    } else {
        return false;
    }

    if(align[1] == 'l') {
        *align_h = AlignLeft;
    } else if(align[1] == 'm') { // m = middle
        *align_h = AlignCenter;
    } else if(align[1] == 'r') {
        *align_h = AlignRight;
    } else {
        return false;
    }

    return true;
}

/**
 * @brief Parses font from an element declaration object
 */
static bool element_get_font(struct mjs* mjs, mjs_val_t element, Font* font) {
    mjs_val_t font_in = mjs_get(mjs, element, "font", ~0);
    const char* font_str = mjs_get_string(mjs, &font_in, NULL);
    if(!font_str) return false;

    if(strcmp(font_str, "primary") == 0) {
        *font = FontPrimary;
    } else if(strcmp(font_str, "secondary") == 0) {
        *font = FontSecondary;
    } else if(strcmp(font_str, "keyboard") == 0) {
        *font = FontKeyboard;
    } else if(strcmp(font_str, "big_numbers") == 0) {
        *font = FontBigNumbers;
    } else {
        return false;
    }
    return true;
}

/**
 * @brief Parses text from an element declaration object
 */
static bool element_get_text(struct mjs* mjs, mjs_val_t element, mjs_val_t* text) {
    *text = mjs_get(mjs, element, "text", ~0);
    return mjs_is_string(*text);
}

/**
 * @brief Widget button element callback
 */
static void js_widget_button_callback(GuiButtonType result, InputType type, JsWidgetCtx* context) {
    UNUSED(type);
    furi_check(furi_message_queue_put(context->queue, &result, 0) == FuriStatusOk);
}

#define DESTRUCTURE_OR_RETURN(mjs, child_obj, part, ...) \
    if(!element_get_##part(mjs, child_obj, __VA_ARGS__)) \
        JS_ERROR_AND_RETURN_VAL(mjs, MJS_BAD_ARGS_ERROR, false, "failed to fetch element " #part);

static bool js_widget_add_child(
    struct mjs* mjs,
    Widget* widget,
    JsWidgetCtx* context,
    mjs_val_t child_obj) {
    UNUSED(context);
    if(!mjs_is_object(child_obj))
        JS_ERROR_AND_RETURN_VAL(mjs, MJS_BAD_ARGS_ERROR, false, "child must be an object");

    mjs_val_t element_type_term = mjs_get(mjs, child_obj, "element", ~0);
    const char* element_type = mjs_get_string(mjs, &element_type_term, NULL);
    if(!element_type)
        JS_ERROR_AND_RETURN_VAL(
            mjs, MJS_BAD_ARGS_ERROR, false, "child object must have `element` property");

    if((strcmp(element_type, "string") == 0) || (strcmp(element_type, "string_multiline") == 0)) {
        int32_t x, y;
        Align align_v, align_h;
        Font font;
        mjs_val_t text;
        DESTRUCTURE_OR_RETURN(mjs, child_obj, position, &x, &y);
        DESTRUCTURE_OR_RETURN(mjs, child_obj, alignment, &align_v, &align_h);
        DESTRUCTURE_OR_RETURN(mjs, child_obj, font, &font);
        DESTRUCTURE_OR_RETURN(mjs, child_obj, text, &text);
        if(strcmp(element_type, "string") == 0) {
            widget_add_string_element(
                widget, x, y, align_h, align_v, font, mjs_get_string(mjs, &text, NULL));
        } else {
            widget_add_string_multiline_element(
                widget, x, y, align_h, align_v, font, mjs_get_string(mjs, &text, NULL));
        }

    } else if(strcmp(element_type, "text_box") == 0) {
        int32_t x, y, w, h;
        Align align_v, align_h;
        Font font;
        mjs_val_t text;
        DESTRUCTURE_OR_RETURN(mjs, child_obj, position, &x, &y);
        DESTRUCTURE_OR_RETURN(mjs, child_obj, size, &w, &h);
        DESTRUCTURE_OR_RETURN(mjs, child_obj, alignment, &align_v, &align_h);
        DESTRUCTURE_OR_RETURN(mjs, child_obj, font, &font);
        DESTRUCTURE_OR_RETURN(mjs, child_obj, text, &text);
        mjs_val_t strip_to_dots_in = mjs_get(mjs, child_obj, "stripToDots", ~0);
        if(!mjs_is_boolean(strip_to_dots_in))
            JS_ERROR_AND_RETURN_VAL(
                mjs, MJS_BAD_ARGS_ERROR, false, "failed to fetch element stripToDots");
        bool strip_to_dots = mjs_get_bool(mjs, strip_to_dots_in);
        widget_add_text_box_element(
            widget, x, y, w, h, align_h, align_v, mjs_get_string(mjs, &text, NULL), strip_to_dots);

    } else if(strcmp(element_type, "text_scroll") == 0) {
        int32_t x, y, w, h;
        mjs_val_t text;
        DESTRUCTURE_OR_RETURN(mjs, child_obj, position, &x, &y);
        DESTRUCTURE_OR_RETURN(mjs, child_obj, size, &w, &h);
        DESTRUCTURE_OR_RETURN(mjs, child_obj, text, &text);
        widget_add_text_scroll_element(widget, x, y, w, h, mjs_get_string(mjs, &text, NULL));

    } else if(strcmp(element_type, "button") == 0) {
        mjs_val_t btn_in = mjs_get(mjs, child_obj, "button", ~0);
        const char* btn_name = mjs_get_string(mjs, &btn_in, NULL);
        if(!btn_name)
            JS_ERROR_AND_RETURN_VAL(
                mjs, MJS_BAD_ARGS_ERROR, false, "failed to fetch element button");
        GuiButtonType btn_type;
        if(strcmp(btn_name, "left") == 0) {
            btn_type = GuiButtonTypeLeft;
        } else if(strcmp(btn_name, "center") == 0) {
            btn_type = GuiButtonTypeCenter;
        } else if(strcmp(btn_name, "right") == 0) {
            btn_type = GuiButtonTypeRight;
        } else {
            JS_ERROR_AND_RETURN_VAL(mjs, MJS_BAD_ARGS_ERROR, false, "incorrect button type");
        }
        mjs_val_t text;
        DESTRUCTURE_OR_RETURN(mjs, child_obj, text, &text);
        widget_add_button_element(
            widget,
            btn_type,
            mjs_get_string(mjs, &text, NULL),
            (ButtonCallback)js_widget_button_callback,
            context);

    } else if(strcmp(element_type, "icon") == 0) {
        int32_t x, y;
        DESTRUCTURE_OR_RETURN(mjs, child_obj, position, &x, &y);
        mjs_val_t icon_data_in = mjs_get(mjs, child_obj, "iconData", ~0);
        if(!mjs_is_foreign(icon_data_in))
            JS_ERROR_AND_RETURN_VAL(
                mjs, MJS_BAD_ARGS_ERROR, false, "failed to fetch element iconData");
        const Icon* icon = mjs_get_ptr(mjs, icon_data_in);
        widget_add_icon_element(widget, x, y, icon);

    } else if(strcmp(element_type, "rect") == 0) {
        int32_t x, y, w, h;
        DESTRUCTURE_OR_RETURN(mjs, child_obj, position, &x, &y);
        DESTRUCTURE_OR_RETURN(mjs, child_obj, size, &w, &h);
        mjs_val_t radius_in = mjs_get(mjs, child_obj, "radius", ~0);
        if(!mjs_is_number(radius_in))
            JS_ERROR_AND_RETURN_VAL(
                mjs, MJS_BAD_ARGS_ERROR, false, "failed to fetch element radius");
        int32_t radius = mjs_get_int32(mjs, radius_in);
        mjs_val_t fill_in = mjs_get(mjs, child_obj, "fill", ~0);
        if(!mjs_is_boolean(fill_in))
            JS_ERROR_AND_RETURN_VAL(
                mjs, MJS_BAD_ARGS_ERROR, false, "failed to fetch element fill");
        int32_t fill = mjs_get_bool(mjs, fill_in);
        widget_add_rect_element(widget, x, y, w, h, radius, fill);

    } else if(strcmp(element_type, "circle") == 0) {
        int32_t x, y;
        DESTRUCTURE_OR_RETURN(mjs, child_obj, position, &x, &y);
        mjs_val_t radius_in = mjs_get(mjs, child_obj, "radius", ~0);
        if(!mjs_is_number(radius_in))
            JS_ERROR_AND_RETURN_VAL(
                mjs, MJS_BAD_ARGS_ERROR, false, "failed to fetch element radius");
        int32_t radius = mjs_get_int32(mjs, radius_in);
        mjs_val_t fill_in = mjs_get(mjs, child_obj, "fill", ~0);
        if(!mjs_is_boolean(fill_in))
            JS_ERROR_AND_RETURN_VAL(
                mjs, MJS_BAD_ARGS_ERROR, false, "failed to fetch element fill");
        int32_t fill = mjs_get_bool(mjs, fill_in);
        widget_add_circle_element(widget, x, y, radius, fill);

    } else if(strcmp(element_type, "line") == 0) {
        int32_t x1, y1, x2, y2;
        mjs_val_t x1_in = mjs_get(mjs, child_obj, "x1", ~0);
        mjs_val_t y1_in = mjs_get(mjs, child_obj, "y1", ~0);
        mjs_val_t x2_in = mjs_get(mjs, child_obj, "x2", ~0);
        mjs_val_t y2_in = mjs_get(mjs, child_obj, "y2", ~0);
        if(!mjs_is_number(x1_in) || !mjs_is_number(y1_in) || !mjs_is_number(x2_in) ||
           !mjs_is_number(y2_in))
            JS_ERROR_AND_RETURN_VAL(
                mjs, MJS_BAD_ARGS_ERROR, false, "failed to fetch element positions");
        x1 = mjs_get_int32(mjs, x1_in);
        y1 = mjs_get_int32(mjs, y1_in);
        x2 = mjs_get_int32(mjs, x2_in);
        y2 = mjs_get_int32(mjs, y2_in);
        widget_add_line_element(widget, x1, y1, x2, y2);
    }

    return true;
}

static void js_widget_reset_children(Widget* widget, void* state) {
    UNUSED(state);
    widget_reset(widget);
}

static mjs_val_t js_widget_button_event_transformer(
    struct mjs* mjs,
    FuriMessageQueue* queue,
    JsWidgetCtx* context) {
    UNUSED(context);
    GuiButtonType btn_type;
    furi_check(furi_message_queue_get(queue, &btn_type, 0) == FuriStatusOk);
    const char* btn_name;
    if(btn_type == GuiButtonTypeLeft) {
        btn_name = "left";
    } else if(btn_type == GuiButtonTypeCenter) {
        btn_name = "center";
    } else if(btn_type == GuiButtonTypeRight) {
        btn_name = "right";
    } else {
        furi_crash();
    }
    return mjs_mk_string(mjs, btn_name, ~0, false);
}

static void* js_widget_custom_make(struct mjs* mjs, Widget* widget, mjs_val_t view_obj) {
    UNUSED(widget);
    JsWidgetCtx* context = malloc(sizeof(JsWidgetCtx));
    context->queue = furi_message_queue_alloc(QUEUE_LEN, sizeof(GuiButtonType));
    context->contract = (JsEventLoopContract){
        .magic = JsForeignMagic_JsEventLoopContract,
        .object_type = JsEventLoopObjectTypeQueue,
        .object = context->queue,
        .non_timer =
            {
                .event = FuriEventLoopEventIn,
                .transformer = (JsEventLoopTransformer)js_widget_button_event_transformer,
            },
    };
    mjs_set(mjs, view_obj, "button", ~0, mjs_mk_foreign(mjs, &context->contract));
    return context;
}

static void js_widget_custom_destroy(Widget* widget, JsWidgetCtx* context, FuriEventLoop* loop) {
    UNUSED(widget);
    furi_event_loop_maybe_unsubscribe(loop, context->queue);
    furi_message_queue_free(context->queue);
    free(context);
}

static const JsViewDescriptor view_descriptor = {
    .alloc = (JsViewAlloc)widget_alloc,
    .free = (JsViewFree)widget_free,
    .get_view = (JsViewGetView)widget_get_view,
    .custom_make = (JsViewCustomMake)js_widget_custom_make,
    .custom_destroy = (JsViewCustomDestroy)js_widget_custom_destroy,
    .add_child = (JsViewAddChild)js_widget_add_child,
    .reset_children = (JsViewResetChildren)js_widget_reset_children,
    .prop_cnt = 0,
    .props = {},
};
JS_GUI_VIEW_DEF(widget, &view_descriptor);
