#include <assets_icons.h>
#include <gui/view_holder.h>
#include <m-array.h>
#include <m-list.h>
#include <string.h>
#include "../js_modules.h"

typedef struct WidgetComponent WidgetComponent;
ARRAY_DEF(ComponentArray, WidgetComponent*, M_PTR_OPLIST);

typedef struct XbmImage XbmImage;
LIST_DEF(XbmImageList, XbmImage*, M_POD_OPLIST);

struct WidgetComponent {
    void (*draw)(Canvas* canvas, void* model);
    void (*free)(WidgetComponent* component);
    void* model;
    uint32_t id;
};

struct XbmImage {
    uint32_t width;
    uint32_t height;
    uint8_t data[];
};

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t w;
    uint8_t h;
} BoxElement;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t r;
} CircleElement;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t r;
} DiscElement;

typedef struct {
    uint8_t x;
    uint8_t y;
} DotElement;

typedef struct {
    uint8_t x;
    uint8_t y;
    const Icon* icon;
} IconElement;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t w;
    uint8_t h;
} FrameElement;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint16_t ch;
} GlyphElement;

typedef struct {
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
} LineElement;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t w;
    uint8_t h;
    uint8_t r;
} RboxElement;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t w;
    uint8_t h;
    uint8_t r;
} RframeElement;

typedef struct {
    uint8_t x;
    uint8_t y;
    Font font;
    FuriString* text;
} TextElement;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint32_t index;
    View* view;
} XbmElement;

typedef struct {
    ComponentArray_t component;
    XbmImageList_t image;
    uint32_t max_assigned_id;
} WidgetModel;

typedef struct {
    View* view;
    ViewHolder* view_holder;
    bool is_shown;
} JsWidgetInst;

static JsWidgetInst* get_this_ctx(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsWidgetInst* widget = mjs_get_ptr(mjs, obj_inst);
    furi_assert(widget);
    return widget;
}

static void ret_bad_args(struct mjs* mjs, const char* error) {
    mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "%s", error);
    mjs_return(mjs, MJS_UNDEFINED);
}

static bool check_arg_count(struct mjs* mjs, size_t count) {
    size_t num_args = mjs_nargs(mjs);
    if(num_args != count) {
        ret_bad_args(mjs, "Wrong argument count");
        return false;
    }
    return true;
}

static void js_widget_load_image_xbm(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 1)) {
        return;
    }

    mjs_val_t path_arg = mjs_arg(mjs, 0);
    size_t path_len = 0;
    const char* path = mjs_get_string(mjs, &path_arg, &path_len);
    if(!path) {
        ret_bad_args(mjs, "Path must be a string");
        return;
    }

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    XbmImage* xbm = NULL;

    do {
        if(!storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            ret_bad_args(mjs, "Failed to open file");
            break;
        }

        uint32_t size = 0;
        if(storage_file_read(file, &size, sizeof(size)) != sizeof(size)) {
            ret_bad_args(mjs, "Failed to get file size");
            break;
        }

        xbm = malloc(size);
        if(storage_file_read(file, xbm, size) != size) {
            ret_bad_args(mjs, "Failed to load entire file");
            free(xbm);
            xbm = NULL;
            break;
        }
    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    if(xbm == NULL) {
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    uint32_t count = 0;
    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            count = XbmImageList_size(model->image);
            XbmImageList_push_back(model->image, xbm);
        },
        false);

    mjs_return(mjs, mjs_mk_number(mjs, count));
}

static void js_widget_remove(struct mjs* mjs) {
    bool removed = false;
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 1)) {
        return;
    }

    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            uint32_t id = mjs_get_int32(mjs, mjs_arg(mjs, 0));
            ComponentArray_it_t it;
            ComponentArray_it(it, model->component);
            while(!ComponentArray_end_p(it)) {
                WidgetComponent* component = *ComponentArray_ref(it);
                if(component->id == id) {
                    if(component->free) {
                        component->free(component);
                    }
                    ComponentArray_remove(model->component, it);
                    removed = true;
                    break;
                }
                ComponentArray_next(it);
            }
        },
        true);

    mjs_return(mjs, mjs_mk_boolean(mjs, removed));
}

static void widget_box_draw(Canvas* canvas, void* model) {
    BoxElement* element = model;
    canvas_draw_box(canvas, element->x, element->y, element->w, element->h);
}

static void widget_box_free(WidgetComponent* component) {
    BoxElement* element = component->model;
    free(element);
    free(component);
}

static void js_widget_add_box(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 4)) return;

    int32_t x = mjs_get_int32(mjs, mjs_arg(mjs, 0));
    int32_t y = mjs_get_int32(mjs, mjs_arg(mjs, 1));
    int32_t w = mjs_get_int32(mjs, mjs_arg(mjs, 2));
    int32_t h = mjs_get_int32(mjs, mjs_arg(mjs, 3));

    WidgetComponent* component = malloc(sizeof(WidgetComponent));
    component->draw = widget_box_draw;
    component->free = widget_box_free;
    component->model = malloc(sizeof(BoxElement));
    BoxElement* element = component->model;
    element->x = x;
    element->y = y;
    element->w = w;
    element->h = h;

    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            ++model->max_assigned_id;
            component->id = model->max_assigned_id;
            ComponentArray_push_back(model->component, component);
        },
        true);

    mjs_return(mjs, mjs_mk_number(mjs, component->id));
}

static void widget_circle_draw(Canvas* canvas, void* model) {
    CircleElement* element = model;
    canvas_draw_circle(canvas, element->x, element->y, element->r);
}

static void widget_circle_free(WidgetComponent* component) {
    CircleElement* element = component->model;
    free(element);
    free(component);
}

static void js_widget_add_circle(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 3)) return;

    int32_t x = mjs_get_int32(mjs, mjs_arg(mjs, 0));
    int32_t y = mjs_get_int32(mjs, mjs_arg(mjs, 1));
    int32_t r = mjs_get_int32(mjs, mjs_arg(mjs, 2));

    WidgetComponent* component = malloc(sizeof(WidgetComponent));
    component->draw = widget_circle_draw;
    component->free = widget_circle_free;
    component->model = malloc(sizeof(CircleElement));
    CircleElement* element = component->model;
    element->x = x;
    element->y = y;
    element->r = r;

    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            ++model->max_assigned_id;
            component->id = model->max_assigned_id;
            ComponentArray_push_back(model->component, component);
        },
        true);

    mjs_return(mjs, mjs_mk_number(mjs, component->id));
}

static void widget_disc_draw(Canvas* canvas, void* model) {
    DiscElement* element = model;
    canvas_draw_disc(canvas, element->x, element->y, element->r);
}

static void widget_disc_free(WidgetComponent* component) {
    DiscElement* element = component->model;
    free(element);
    free(component);
}

static void js_widget_add_disc(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 3)) return;

    int32_t x = mjs_get_int32(mjs, mjs_arg(mjs, 0));
    int32_t y = mjs_get_int32(mjs, mjs_arg(mjs, 1));
    int32_t r = mjs_get_int32(mjs, mjs_arg(mjs, 2));

    WidgetComponent* component = malloc(sizeof(WidgetComponent));
    component->draw = widget_disc_draw;
    component->free = widget_disc_free;
    component->model = malloc(sizeof(DiscElement));
    DiscElement* element = component->model;
    element->x = x;
    element->y = y;
    element->r = r;

    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            ++model->max_assigned_id;
            component->id = model->max_assigned_id;
            ComponentArray_push_back(model->component, component);
        },
        true);

    mjs_return(mjs, mjs_mk_number(mjs, component->id));
}

static void widget_dot_draw(Canvas* canvas, void* model) {
    DotElement* element = model;
    canvas_draw_dot(canvas, element->x, element->y);
}

static void widget_dot_free(WidgetComponent* component) {
    DotElement* element = component->model;
    free(element);
    free(component);
}

static void js_widget_add_dot(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 2)) return;

    int32_t x = mjs_get_int32(mjs, mjs_arg(mjs, 0));
    int32_t y = mjs_get_int32(mjs, mjs_arg(mjs, 1));

    WidgetComponent* component = malloc(sizeof(WidgetComponent));
    component->draw = widget_dot_draw;
    component->free = widget_dot_free;
    component->model = malloc(sizeof(DotElement));
    DotElement* element = component->model;
    element->x = x;
    element->y = y;

    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            ++model->max_assigned_id;
            component->id = model->max_assigned_id;
            ComponentArray_push_back(model->component, component);
        },
        true);

    mjs_return(mjs, mjs_mk_number(mjs, component->id));
}

static void widget_frame_draw(Canvas* canvas, void* model) {
    FrameElement* element = model;
    canvas_draw_frame(canvas, element->x, element->y, element->w, element->h);
}

static void widget_frame_free(WidgetComponent* component) {
    FrameElement* element = component->model;
    free(element);
    free(component);
}

static void js_widget_add_frame(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 4)) return;

    int32_t x = mjs_get_int32(mjs, mjs_arg(mjs, 0));
    int32_t y = mjs_get_int32(mjs, mjs_arg(mjs, 1));
    int32_t w = mjs_get_int32(mjs, mjs_arg(mjs, 2));
    int32_t h = mjs_get_int32(mjs, mjs_arg(mjs, 3));

    WidgetComponent* component = malloc(sizeof(WidgetComponent));
    component->draw = widget_frame_draw;
    component->free = widget_frame_free;
    component->model = malloc(sizeof(FrameElement));
    FrameElement* element = component->model;
    element->x = x;
    element->y = y;
    element->w = w;
    element->h = h;

    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            ++model->max_assigned_id;
            component->id = model->max_assigned_id;
            ComponentArray_push_back(model->component, component);
        },
        true);

    mjs_return(mjs, mjs_mk_number(mjs, component->id));
}

static void widget_glyph_draw(Canvas* canvas, void* model) {
    GlyphElement* element = model;
    canvas_draw_glyph(canvas, element->x, element->y, element->ch);
}

static void widget_glyph_free(WidgetComponent* component) {
    GlyphElement* element = component->model;
    free(element);
    free(component);
}

static void js_widget_add_glyph(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 3)) return;

    int32_t x = mjs_get_int32(mjs, mjs_arg(mjs, 0));
    int32_t y = mjs_get_int32(mjs, mjs_arg(mjs, 1));
    int32_t ch = mjs_get_int32(mjs, mjs_arg(mjs, 2));

    WidgetComponent* component = malloc(sizeof(WidgetComponent));
    component->draw = widget_glyph_draw;
    component->free = widget_glyph_free;
    component->model = malloc(sizeof(GlyphElement));
    GlyphElement* element = component->model;
    element->x = x;
    element->y = y;
    element->ch = ch;

    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            ++model->max_assigned_id;
            component->id = model->max_assigned_id;
            ComponentArray_push_back(model->component, component);
        },
        true);

    mjs_return(mjs, mjs_mk_number(mjs, component->id));
}

static void widget_line_draw(Canvas* canvas, void* model) {
    LineElement* element = model;
    canvas_draw_line(canvas, element->x1, element->y1, element->x2, element->y2);
}

static void widget_line_free(WidgetComponent* component) {
    LineElement* element = component->model;
    free(element);
    free(component);
}

static void js_widget_add_line(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 4)) return;

    int32_t x1 = mjs_get_int32(mjs, mjs_arg(mjs, 0));
    int32_t y1 = mjs_get_int32(mjs, mjs_arg(mjs, 1));
    int32_t x2 = mjs_get_int32(mjs, mjs_arg(mjs, 2));
    int32_t y2 = mjs_get_int32(mjs, mjs_arg(mjs, 3));

    WidgetComponent* component = malloc(sizeof(WidgetComponent));
    component->draw = widget_line_draw;
    component->free = widget_line_free;
    component->model = malloc(sizeof(LineElement));
    LineElement* element = component->model;
    element->x1 = x1;
    element->y1 = y1;
    element->x2 = x2;
    element->y2 = y2;

    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            ++model->max_assigned_id;
            component->id = model->max_assigned_id;
            ComponentArray_push_back(model->component, component);
        },
        true);

    mjs_return(mjs, mjs_mk_number(mjs, component->id));
}

static void widget_rbox_draw(Canvas* canvas, void* model) {
    RboxElement* element = model;
    canvas_draw_rbox(canvas, element->x, element->y, element->w, element->h, element->r);
}

static void widget_rbox_free(WidgetComponent* component) {
    BoxElement* element = component->model;
    free(element);
    free(component);
}

static void js_widget_add_rbox(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 5)) return;

    int32_t x = mjs_get_int32(mjs, mjs_arg(mjs, 0));
    int32_t y = mjs_get_int32(mjs, mjs_arg(mjs, 1));
    int32_t w = mjs_get_int32(mjs, mjs_arg(mjs, 2));
    int32_t h = mjs_get_int32(mjs, mjs_arg(mjs, 3));
    int32_t r = mjs_get_int32(mjs, mjs_arg(mjs, 4));

    WidgetComponent* component = malloc(sizeof(WidgetComponent));
    component->draw = widget_rbox_draw;
    component->free = widget_rbox_free;
    component->model = malloc(sizeof(RboxElement));
    RboxElement* element = component->model;
    element->x = x;
    element->y = y;
    element->w = w;
    element->h = h;
    element->r = r;

    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            ++model->max_assigned_id;
            component->id = model->max_assigned_id;
            ComponentArray_push_back(model->component, component);
        },
        true);

    mjs_return(mjs, mjs_mk_number(mjs, component->id));
}

static void widget_rframe_draw(Canvas* canvas, void* model) {
    RframeElement* element = model;
    canvas_draw_rframe(canvas, element->x, element->y, element->w, element->h, element->r);
}

static void widget_rframe_free(WidgetComponent* component) {
    RframeElement* element = component->model;
    free(element);
    free(component);
}

static void js_widget_add_rframe(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 5)) return;

    int32_t x = mjs_get_int32(mjs, mjs_arg(mjs, 0));
    int32_t y = mjs_get_int32(mjs, mjs_arg(mjs, 1));
    int32_t w = mjs_get_int32(mjs, mjs_arg(mjs, 2));
    int32_t h = mjs_get_int32(mjs, mjs_arg(mjs, 3));
    int32_t r = mjs_get_int32(mjs, mjs_arg(mjs, 4));

    WidgetComponent* component = malloc(sizeof(WidgetComponent));
    component->draw = widget_rframe_draw;
    component->free = widget_rframe_free;
    component->model = malloc(sizeof(RframeElement));
    RframeElement* element = component->model;
    element->x = x;
    element->y = y;
    element->w = w;
    element->h = h;
    element->r = r;

    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            ++model->max_assigned_id;
            component->id = model->max_assigned_id;
            ComponentArray_push_back(model->component, component);
        },
        true);

    mjs_return(mjs, mjs_mk_number(mjs, component->id));
}

static void widget_text_draw(Canvas* canvas, void* model) {
    TextElement* element = model;
    canvas_set_font(canvas, element->font);
    canvas_draw_str(canvas, element->x, element->y, furi_string_get_cstr(element->text));
}

static void widget_text_free(WidgetComponent* component) {
    TextElement* element = component->model;
    furi_string_free(element->text);
    free(element);
    free(component);
}

static void js_widget_add_text(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 4)) return;

    int32_t x = mjs_get_int32(mjs, mjs_arg(mjs, 0));
    int32_t y = mjs_get_int32(mjs, mjs_arg(mjs, 1));

    mjs_val_t font_arg = mjs_arg(mjs, 2);
    size_t font_name_len = 0;
    const char* font_name_text = mjs_get_string(mjs, &font_arg, &font_name_len);
    if(!font_name_text) {
        ret_bad_args(mjs, "Font name must be a string");
        return;
    }

    Font font = FontTotalNumber;
    size_t cmp_str_len = strlen("Primary");
    if(font_name_len == cmp_str_len && strncmp(font_name_text, "Primary", cmp_str_len) == 0) {
        font = FontPrimary;
    } else {
        cmp_str_len = strlen("Secondary");
        if(font_name_len == cmp_str_len &&
           strncmp(font_name_text, "Secondary", cmp_str_len) == 0) {
            font = FontSecondary;
        }
    }
    if(font == FontTotalNumber) {
        ret_bad_args(mjs, "Unknown font name");
        return;
    }

    mjs_val_t text_arg = mjs_arg(mjs, 3);
    size_t text_len = 0;
    const char* text = mjs_get_string(mjs, &text_arg, &text_len);
    if(!text) {
        ret_bad_args(mjs, "Text must be a string");
        return;
    }
    FuriString* text_str = furi_string_alloc_set(text);

    WidgetComponent* component = malloc(sizeof(WidgetComponent));
    component->draw = widget_text_draw;
    component->free = widget_text_free;
    component->model = malloc(sizeof(TextElement));
    TextElement* element = component->model;
    element->x = x;
    element->y = y;
    element->font = font;
    element->text = text_str;

    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            ++model->max_assigned_id;
            component->id = model->max_assigned_id;
            ComponentArray_push_back(model->component, component);
        },
        true);

    mjs_return(mjs, mjs_mk_number(mjs, component->id));
}

static void widget_xbm_draw(Canvas* canvas, void* model) {
    XbmElement* element = model;
    XbmImage* image = NULL;

    with_view_model(
        element->view,
        WidgetModel * widget_model,
        { image = *XbmImageList_get(widget_model->image, element->index); },
        false);

    if(image) {
        canvas_draw_xbm(canvas, element->x, element->y, image->width, image->height, image->data);
    }
}

static void widget_xbm_free(WidgetComponent* component) {
    XbmElement* element = component->model;
    free(element);
    free(component);
}

static void js_widget_add_xbm(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 3)) return;

    int32_t x = mjs_get_int32(mjs, mjs_arg(mjs, 0));
    int32_t y = mjs_get_int32(mjs, mjs_arg(mjs, 1));
    int32_t index = mjs_get_int32(mjs, mjs_arg(mjs, 2));
    with_view_model(
        widget->view,
        WidgetModel * widget_model,
        {
            size_t count = XbmImageList_size(widget_model->image);
            if(index < 0 || index >= (int32_t)count) {
                ret_bad_args(mjs, "Invalid image index");
                return;
            }
        },
        false);

    WidgetComponent* component = malloc(sizeof(WidgetComponent));
    component->draw = widget_xbm_draw;
    component->free = widget_xbm_free;
    component->model = malloc(sizeof(XbmElement));
    XbmElement* element = component->model;
    element->x = x;
    element->y = y;
    element->index = index;

    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            ++model->max_assigned_id;
            component->id = model->max_assigned_id;
            element->view = widget->view;
            ComponentArray_push_back(model->component, component);
        },
        true);

    mjs_return(mjs, mjs_mk_number(mjs, component->id));
}

static void js_widget_is_open(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 0)) return;

    mjs_return(mjs, mjs_mk_boolean(mjs, widget->is_shown));
}

static void widget_callback(void* context, uint32_t arg) {
    UNUSED(arg);
    JsWidgetInst* widget = context;
    view_holder_stop(widget->view_holder);
    widget->is_shown = false;
}

static void widget_exit(void* context) {
    JsWidgetInst* widget = context;
    // Using timer to schedule view_holder stop, will not work under high CPU load
    furi_timer_pending_callback(widget_callback, widget, 0);
}

static void js_widget_show(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 0)) return;

    if(widget->is_shown) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Widget is already shown");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    view_holder_start(widget->view_holder);
    widget->is_shown = true;

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_widget_close(struct mjs* mjs) {
    JsWidgetInst* widget = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 0)) return;

    view_holder_stop(widget->view_holder);
    widget->is_shown = false;

    mjs_return(mjs, MJS_UNDEFINED);
}

static void widget_draw_callback(Canvas* canvas, void* model) {
    WidgetModel* widget_model = model;
    canvas_clear(canvas);

    ComponentArray_it_t it;
    ComponentArray_it(it, widget_model->component);
    while(!ComponentArray_end_p(it)) {
        WidgetComponent* component = *ComponentArray_ref(it);
        if(component->draw != NULL) {
            component->draw(canvas, component->model);
        }
        ComponentArray_next(it);
    }
}

static void* js_widget_create(struct mjs* mjs, mjs_val_t* object) {
    JsWidgetInst* widget = malloc(sizeof(JsWidgetInst));

    mjs_val_t widget_obj = mjs_mk_object(mjs);
    mjs_set(mjs, widget_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, widget));
    // addBox(x: number, y: number, w: number, h: number): number (returns id of the added component)
    mjs_set(mjs, widget_obj, "addBox", ~0, MJS_MK_FN(js_widget_add_box));
    // addCircle(x: number, y: number, r: number): number (returns id of the added component)
    mjs_set(mjs, widget_obj, "addCircle", ~0, MJS_MK_FN(js_widget_add_circle));
    // addDisc(x: number, y: number, r: number): number (returns id of the added component)
    mjs_set(mjs, widget_obj, "addDisc", ~0, MJS_MK_FN(js_widget_add_disc));
    // addDot(x: number, y: number): number (returns id of the added component)
    mjs_set(mjs, widget_obj, "addDot", ~0, MJS_MK_FN(js_widget_add_dot));
    // addFrame(x: number, y: number, w: number, h: number): number (returns id of the added component)
    mjs_set(mjs, widget_obj, "addFrame", ~0, MJS_MK_FN(js_widget_add_frame));
    // addGlyph(x: number, y: number, ch: number): number (returns id of the added component)
    mjs_set(mjs, widget_obj, "addGlyph", ~0, MJS_MK_FN(js_widget_add_glyph));
    // addLine(x1: number, y1: number, x2: number, y2: number): number (returns id of the added component)
    mjs_set(mjs, widget_obj, "addLine", ~0, MJS_MK_FN(js_widget_add_line));
    // addRbox(x: number, y: number, w: number, h: number, r: number): number (returns id of the added component)
    mjs_set(mjs, widget_obj, "addRbox", ~0, MJS_MK_FN(js_widget_add_rbox));
    // addRframe(x: number, y: number, w: number, h: number, r: number): number (returns id of the added component)
    mjs_set(mjs, widget_obj, "addRframe", ~0, MJS_MK_FN(js_widget_add_rframe));
    // addText(x: number, y: number, font: string, text: string): number (returns id of the added component)
    mjs_set(mjs, widget_obj, "addText", ~0, MJS_MK_FN(js_widget_add_text));
    // addXbm(x: number, y: number, index: number): number (returns id of the added component)
    mjs_set(mjs, widget_obj, "addXbm", ~0, MJS_MK_FN(js_widget_add_xbm));
    // loadImageXbm(path: string): number (returns index of the loaded image)
    mjs_set(mjs, widget_obj, "loadImageXbm", ~0, MJS_MK_FN(js_widget_load_image_xbm));
    // remove(id: number): boolean (returns true if the component was removed)
    mjs_set(mjs, widget_obj, "remove", ~0, MJS_MK_FN(js_widget_remove));
    // isOpen(): boolean (returns true if the widget is open)
    mjs_set(mjs, widget_obj, "isOpen", ~0, MJS_MK_FN(js_widget_is_open));
    // show(): void (shows the widget)
    mjs_set(mjs, widget_obj, "show", ~0, MJS_MK_FN(js_widget_show));
    // close(): void (closes the widget)
    mjs_set(mjs, widget_obj, "close", ~0, MJS_MK_FN(js_widget_close));

    widget->view = view_alloc();
    view_allocate_model(widget->view, ViewModelTypeLockFree, sizeof(WidgetModel));
    view_set_draw_callback(widget->view, widget_draw_callback);
    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            ComponentArray_init(model->component);
            XbmImageList_init(model->image);
            model->max_assigned_id = 0;
        },
        true);

    Gui* gui = furi_record_open(RECORD_GUI);
    widget->view_holder = view_holder_alloc();
    view_holder_attach_to_gui(widget->view_holder, gui);
    view_holder_set_back_callback(widget->view_holder, widget_exit, widget);
    view_holder_set_view(widget->view_holder, widget->view);

    *object = widget_obj;
    return widget;
}

static void js_widget_destroy(void* inst) {
    JsWidgetInst* widget = inst;

    view_holder_stop(widget->view_holder);
    view_holder_free(widget->view_holder);
    widget->view_holder = NULL;

    furi_record_close(RECORD_GUI);

    with_view_model(
        widget->view,
        WidgetModel * model,
        {
            ComponentArray_it_t it;
            ComponentArray_it(it, model->component);
            while(!ComponentArray_end_p(it)) {
                WidgetComponent* component = *ComponentArray_ref(it);
                if(component && component->free) {
                    component->free(component);
                }
                ComponentArray_next(it);
            }
            ComponentArray_reset(model->component);
            ComponentArray_clear(model->component);
            XbmImageList_clear(model->image);
        },
        false);
    view_free(widget->view);
    widget->view = NULL;

    free(widget);
}

static const JsModuleDescriptor js_widget_desc = {
    "widget",
    js_widget_create,
    js_widget_destroy,
};

static const FlipperAppPluginDescriptor widget_plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_widget_desc,
};

const FlipperAppPluginDescriptor* js_widget_ep(void) {
    return &widget_plugin_descriptor;
}
