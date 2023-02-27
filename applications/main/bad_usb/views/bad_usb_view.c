#include "bad_usb_view.h"
#include "../bad_usb_script.h"
#include <toolbox/path.h>
#include <gui/elements.h>
#include <assets_icons.h>

#define MAX_NAME_LEN 64

struct BadUsb {
    View* view;
    BadUsbButtonCallback callback;
    void* context;
};

typedef struct {
    char file_name[MAX_NAME_LEN];
    char layout[MAX_NAME_LEN];
    BadUsbState state;
    uint8_t anim_frame;
} BadUsbModel;

static void bad_usb_draw_callback(Canvas* canvas, void* _model) {
    BadUsbModel* model = _model;

    FuriString* disp_str;
    disp_str = furi_string_alloc_set(model->file_name);
    elements_string_fit_width(canvas, disp_str, 128 - 2);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 8, furi_string_get_cstr(disp_str));

    if(strlen(model->layout) == 0) {
        furi_string_set(disp_str, "(default)");
    } else {
        furi_string_reset(disp_str);
        furi_string_push_back(disp_str, '(');
        for(size_t i = 0; i < strlen(model->layout); i++)
            furi_string_push_back(disp_str, model->layout[i]);
        furi_string_push_back(disp_str, ')');
    }
    elements_string_fit_width(canvas, disp_str, 128 - 2);
    canvas_draw_str(
        canvas, 2, 8 + canvas_current_font_height(canvas), furi_string_get_cstr(disp_str));

    furi_string_reset(disp_str);

    canvas_draw_icon(canvas, 22, 24, &I_UsbTree_48x22);

    if((model->state.state == BadUsbStateIdle) || (model->state.state == BadUsbStateDone) ||
       (model->state.state == BadUsbStateNotConnected)) {
        elements_button_center(canvas, "Run");
        elements_button_left(canvas, "Config");
    } else if((model->state.state == BadUsbStateRunning) || (model->state.state == BadUsbStateDelay)) {
        elements_button_center(canvas, "Stop");
    } else if(model->state.state == BadUsbStateWillRun) {
        elements_button_center(canvas, "Cancel");
    }

    if(model->state.state == BadUsbStateNotConnected) {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "Connect");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "to USB");
    } else if(model->state.state == BadUsbStateWillRun) {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "Will run");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "on connect");
    } else if(model->state.state == BadUsbStateFileError) {
        canvas_draw_icon(canvas, 4, 26, &I_Error_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 31, AlignRight, AlignBottom, "File");
        canvas_draw_str_aligned(canvas, 127, 43, AlignRight, AlignBottom, "ERROR");
    } else if(model->state.state == BadUsbStateScriptError) {
        canvas_draw_icon(canvas, 4, 26, &I_Error_18x18);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 127, 33, AlignRight, AlignBottom, "ERROR:");
        canvas_set_font(canvas, FontSecondary);
        furi_string_printf(disp_str, "line %u", model->state.error_line);
        canvas_draw_str_aligned(
            canvas, 127, 46, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
        canvas_draw_str_aligned(canvas, 127, 56, AlignRight, AlignBottom, model->state.error);
    } else if(model->state.state == BadUsbStateIdle) {
        canvas_draw_icon(canvas, 4, 26, &I_Smile_18x18);
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 114, 40, AlignRight, AlignBottom, "0");
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
    } else if(model->state.state == BadUsbStateRunning) {
        if(model->anim_frame == 0) {
            canvas_draw_icon(canvas, 4, 23, &I_EviSmile1_18x21);
        } else {
            canvas_draw_icon(canvas, 4, 23, &I_EviSmile2_18x21);
        }
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(
            disp_str, "%u", ((model->state.line_cur - 1) * 100) / model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 114, 40, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
    } else if(model->state.state == BadUsbStateDone) {
        canvas_draw_icon(canvas, 4, 23, &I_EviSmile1_18x21);
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 114, 40, AlignRight, AlignBottom, "100");
        furi_string_reset(disp_str);
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
    } else if(model->state.state == BadUsbStateDelay) {
        if(model->anim_frame == 0) {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting1_18x21);
        } else {
            canvas_draw_icon(canvas, 4, 23, &I_EviWaiting2_18x21);
        }
        canvas_set_font(canvas, FontBigNumbers);
        furi_string_printf(
            disp_str, "%u", ((model->state.line_cur - 1) * 100) / model->state.line_nb);
        canvas_draw_str_aligned(
            canvas, 114, 40, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
        canvas_draw_icon(canvas, 117, 26, &I_Percent_10x14);
        canvas_set_font(canvas, FontSecondary);
        furi_string_printf(disp_str, "delay %lus", model->state.delay_remain);
        canvas_draw_str_aligned(
            canvas, 127, 50, AlignRight, AlignBottom, furi_string_get_cstr(disp_str));
        furi_string_reset(disp_str);
    } else {
        canvas_draw_icon(canvas, 4, 26, &I_Clock_18x18);
    }

    furi_string_free(disp_str);
}

static bool bad_usb_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    BadUsb* bad_usb = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if((event->key == InputKeyLeft) || (event->key == InputKeyOk)) {
            consumed = true;
            furi_assert(bad_usb->callback);
            bad_usb->callback(event->key, bad_usb->context);
        }
    }

    return consumed;
}

BadUsb* bad_usb_alloc() {
    BadUsb* bad_usb = malloc(sizeof(BadUsb));

    bad_usb->view = view_alloc();
    view_allocate_model(bad_usb->view, ViewModelTypeLocking, sizeof(BadUsbModel));
    view_set_context(bad_usb->view, bad_usb);
    view_set_draw_callback(bad_usb->view, bad_usb_draw_callback);
    view_set_input_callback(bad_usb->view, bad_usb_input_callback);

    return bad_usb;
}

void bad_usb_free(BadUsb* bad_usb) {
    furi_assert(bad_usb);
    view_free(bad_usb->view);
    free(bad_usb);
}

View* bad_usb_get_view(BadUsb* bad_usb) {
    furi_assert(bad_usb);
    return bad_usb->view;
}

void bad_usb_set_button_callback(BadUsb* bad_usb, BadUsbButtonCallback callback, void* context) {
    furi_assert(bad_usb);
    furi_assert(callback);
    with_view_model(
        bad_usb->view,
        BadUsbModel * model,
        {
            UNUSED(model);
            bad_usb->callback = callback;
            bad_usb->context = context;
        },
        true);
}

void bad_usb_set_file_name(BadUsb* bad_usb, const char* name) {
    furi_assert(name);
    with_view_model(
        bad_usb->view,
        BadUsbModel * model,
        { strlcpy(model->file_name, name, MAX_NAME_LEN); },
        true);
}

void bad_usb_set_layout(BadUsb* bad_usb, const char* layout) {
    furi_assert(layout);
    with_view_model(
        bad_usb->view,
        BadUsbModel * model,
        { strlcpy(model->layout, layout, MAX_NAME_LEN); },
        true);
}

void bad_usb_set_state(BadUsb* bad_usb, BadUsbState* st) {
    furi_assert(st);
    with_view_model(
        bad_usb->view,
        BadUsbModel * model,
        {
            memcpy(&(model->state), st, sizeof(BadUsbState));
            model->anim_frame ^= 1;
        },
        true);
}

bool bad_usb_is_idle_state(BadUsb* bad_usb) {
    bool is_idle = false;
    with_view_model(
        bad_usb->view,
        BadUsbModel * model,
        {
            if((model->state.state == BadUsbStateIdle) ||
               (model->state.state == BadUsbStateDone) ||
               (model->state.state == BadUsbStateNotConnected)) {
                is_idle = true;
            }
        },
        false);
    return is_idle;
}
