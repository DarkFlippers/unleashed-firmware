#include "pocsag_pager_receiver.h"
#include "../pocsag_pager_app_i.h"
#include "pocsag_pager_icons.h"
#include "../protocols/pcsg_generic.h"
#include <input/input.h>
#include <gui/elements.h>

#define abs(x) ((x) > 0 ? (x) : -(x))

struct PCSGReceiverInfo {
    View* view;
};

typedef struct {
    FuriString* protocol_name;
    PCSGBlockGeneric* generic;
} PCSGReceiverInfoModel;

void pcsg_view_receiver_info_update(PCSGReceiverInfo* pcsg_receiver_info, FlipperFormat* fff) {
    furi_assert(pcsg_receiver_info);
    furi_assert(fff);

    with_view_model(
        pcsg_receiver_info->view,
        PCSGReceiverInfoModel * model,
        {
            flipper_format_rewind(fff);
            flipper_format_read_string(fff, "Protocol", model->protocol_name);

            pcsg_block_generic_deserialize(model->generic, fff);
        },
        true);
}

void pcsg_view_receiver_info_draw(Canvas* canvas, PCSGReceiverInfoModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    if(model->generic->result_ric != NULL) {
        elements_text_box(
            canvas,
            0,
            0,
            128,
            64,
            AlignLeft,
            AlignTop,
            furi_string_get_cstr(model->generic->result_ric),
            false);
    }
    if(model->generic->result_msg != NULL) {
        elements_text_box(
            canvas,
            0,
            12,
            128,
            64,
            AlignLeft,
            AlignTop,
            furi_string_get_cstr(model->generic->result_msg),
            false);
    }
}

bool pcsg_view_receiver_info_input(InputEvent* event, void* context) {
    furi_assert(context);
    //PCSGReceiverInfo* pcsg_receiver_info = context;

    if(event->key == InputKeyBack) {
        return false;
    }

    return true;
}

void pcsg_view_receiver_info_enter(void* context) {
    furi_assert(context);
}

void pcsg_view_receiver_info_exit(void* context) {
    furi_assert(context);
    PCSGReceiverInfo* pcsg_receiver_info = context;

    with_view_model(
        pcsg_receiver_info->view,
        PCSGReceiverInfoModel * model,
        { furi_string_reset(model->protocol_name); },
        false);
}

PCSGReceiverInfo* pcsg_view_receiver_info_alloc() {
    PCSGReceiverInfo* pcsg_receiver_info = malloc(sizeof(PCSGReceiverInfo));

    // View allocation and configuration
    pcsg_receiver_info->view = view_alloc();

    view_allocate_model(
        pcsg_receiver_info->view, ViewModelTypeLocking, sizeof(PCSGReceiverInfoModel));
    view_set_context(pcsg_receiver_info->view, pcsg_receiver_info);
    view_set_draw_callback(
        pcsg_receiver_info->view, (ViewDrawCallback)pcsg_view_receiver_info_draw);
    view_set_input_callback(pcsg_receiver_info->view, pcsg_view_receiver_info_input);
    view_set_enter_callback(pcsg_receiver_info->view, pcsg_view_receiver_info_enter);
    view_set_exit_callback(pcsg_receiver_info->view, pcsg_view_receiver_info_exit);

    with_view_model(
        pcsg_receiver_info->view,
        PCSGReceiverInfoModel * model,
        {
            model->generic = malloc(sizeof(PCSGBlockGeneric));
            model->protocol_name = furi_string_alloc();
        },
        true);

    return pcsg_receiver_info;
}

void pcsg_view_receiver_info_free(PCSGReceiverInfo* pcsg_receiver_info) {
    furi_assert(pcsg_receiver_info);

    with_view_model(
        pcsg_receiver_info->view,
        PCSGReceiverInfoModel * model,
        {
            furi_string_free(model->protocol_name);
            free(model->generic);
        },
        false);

    view_free(pcsg_receiver_info->view);
    free(pcsg_receiver_info);
}

View* pcsg_view_receiver_info_get_view(PCSGReceiverInfo* pcsg_receiver_info) {
    furi_assert(pcsg_receiver_info);
    return pcsg_receiver_info->view;
}
