#include "dap_main_view.h"
#include "dap_link_icons.h"
#include <gui/elements.h>

// extern const Icon I_ArrowDownEmpty_12x18;
// extern const Icon I_ArrowDownFilled_12x18;
// extern const Icon I_ArrowUpEmpty_12x18;
// extern const Icon I_ArrowUpFilled_12x18;

struct DapMainView {
    View* view;
    DapMainViewButtonCallback cb_left;
    void* cb_context;
};

typedef struct {
    DapMainViewMode mode;
    DapMainViewVersion version;
    bool usb_connected;
    uint32_t baudrate;
    bool dap_active;
    bool tx_active;
    bool rx_active;
} DapMainViewModel;

static void dap_main_view_draw_callback(Canvas* canvas, void* _model) {
    DapMainViewModel* model = _model;
    UNUSED(model);
    canvas_clear(canvas);
    elements_button_left(canvas, "Config");

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_box(canvas, 0, 0, 127, 11);
    canvas_set_color(canvas, ColorWhite);

    const char* header_string;
    if(model->usb_connected) {
        if(model->version == DapMainViewVersionV1) {
            header_string = "DAP Link V1 Connected";
        } else if(model->version == DapMainViewVersionV2) {
            header_string = "DAP Link V2 Connected";
        } else {
            header_string = "DAP Link Connected";
        }
    } else {
        header_string = "DAP Link";
    }

    canvas_draw_str_aligned(canvas, 64, 9, AlignCenter, AlignBottom, header_string);

    canvas_set_color(canvas, ColorBlack);
    if(model->dap_active) {
        canvas_draw_icon(canvas, 14, 16, &I_ArrowUpFilled_12x18);
        canvas_draw_icon_ex(canvas, 28, 16, &I_ArrowUpFilled_12x18, IconRotation180);
    } else {
        canvas_draw_icon(canvas, 14, 16, &I_ArrowUpEmpty_12x18);
        canvas_draw_icon_ex(canvas, 28, 16, &I_ArrowUpEmpty_12x18, IconRotation180);
    }

    switch(model->mode) {
    case DapMainViewModeDisconnected:
        canvas_draw_str_aligned(canvas, 26, 38, AlignCenter, AlignTop, "----");
        break;
    case DapMainViewModeSWD:
        canvas_draw_str_aligned(canvas, 26, 38, AlignCenter, AlignTop, "SWD");
        break;
    case DapMainViewModeJTAG:
        canvas_draw_str_aligned(canvas, 26, 38, AlignCenter, AlignTop, "JTAG");
        break;
    }

    if(model->tx_active) {
        canvas_draw_icon(canvas, 87, 16, &I_ArrowUpFilled_12x18);
    } else {
        canvas_draw_icon(canvas, 87, 16, &I_ArrowUpEmpty_12x18);
    }

    if(model->rx_active) {
        canvas_draw_icon_ex(canvas, 101, 16, &I_ArrowUpFilled_12x18, IconRotation180);
    } else {
        canvas_draw_icon_ex(canvas, 101, 16, &I_ArrowUpEmpty_12x18, IconRotation180);
    }

    canvas_draw_str_aligned(canvas, 100, 38, AlignCenter, AlignTop, "UART");

    canvas_draw_line(canvas, 44, 52, 123, 52);
    if(model->baudrate == 0) {
        canvas_draw_str(canvas, 45, 62, "Baud: ????");
    } else {
        char baudrate_str[18];
        snprintf(baudrate_str, 18, "Baud: %lu", model->baudrate);
        canvas_draw_str(canvas, 45, 62, baudrate_str);
    }
}

static bool dap_main_view_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    DapMainView* dap_main_view = context;
    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyLeft) {
            if(dap_main_view->cb_left) {
                dap_main_view->cb_left(dap_main_view->cb_context);
            }
            consumed = true;
        }
    }

    return consumed;
}

DapMainView* dap_main_view_alloc() {
    DapMainView* dap_main_view = malloc(sizeof(DapMainView));

    dap_main_view->view = view_alloc();
    view_allocate_model(dap_main_view->view, ViewModelTypeLocking, sizeof(DapMainViewModel));
    view_set_context(dap_main_view->view, dap_main_view);
    view_set_draw_callback(dap_main_view->view, dap_main_view_draw_callback);
    view_set_input_callback(dap_main_view->view, dap_main_view_input_callback);
    return dap_main_view;
}

void dap_main_view_free(DapMainView* dap_main_view) {
    view_free(dap_main_view->view);
    free(dap_main_view);
}

View* dap_main_view_get_view(DapMainView* dap_main_view) {
    return dap_main_view->view;
}

void dap_main_view_set_left_callback(
    DapMainView* dap_main_view,
    DapMainViewButtonCallback callback,
    void* context) {
    with_view_model(
        dap_main_view->view,
        DapMainViewModel * model,
        {
            UNUSED(model);
            dap_main_view->cb_left = callback;
            dap_main_view->cb_context = context;
        },
        true);
}

void dap_main_view_set_mode(DapMainView* dap_main_view, DapMainViewMode mode) {
    with_view_model(
        dap_main_view->view, DapMainViewModel * model, { model->mode = mode; }, false);
}

void dap_main_view_set_dap(DapMainView* dap_main_view, bool active) {
    with_view_model(
        dap_main_view->view, DapMainViewModel * model, { model->dap_active = active; }, false);
}

void dap_main_view_set_tx(DapMainView* dap_main_view, bool active) {
    with_view_model(
        dap_main_view->view, DapMainViewModel * model, { model->tx_active = active; }, false);
}

void dap_main_view_set_rx(DapMainView* dap_main_view, bool active) {
    with_view_model(
        dap_main_view->view, DapMainViewModel * model, { model->rx_active = active; }, false);
}

void dap_main_view_set_baudrate(DapMainView* dap_main_view, uint32_t baudrate) {
    with_view_model(
        dap_main_view->view, DapMainViewModel * model, { model->baudrate = baudrate; }, false);
}

void dap_main_view_update(DapMainView* dap_main_view) {
    with_view_model(
        dap_main_view->view, DapMainViewModel * model, { UNUSED(model); }, true);
}

void dap_main_view_set_version(DapMainView* dap_main_view, DapMainViewVersion version) {
    with_view_model(
        dap_main_view->view, DapMainViewModel * model, { model->version = version; }, false);
}

void dap_main_view_set_usb_connected(DapMainView* dap_main_view, bool connected) {
    with_view_model(
        dap_main_view->view,
        DapMainViewModel * model,
        { model->usb_connected = connected; },
        false);
}