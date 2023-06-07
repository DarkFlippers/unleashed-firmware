#include "main_menu.h"
#include "../fuzzer_i.h"

#include <input/input.h>

#include "../lib/worker/protocol.h"

#define PROTOCOL_NAME_Y 12
// #define PROTOCOL_CAROUSEL

struct FuzzerViewMain {
    View* view;
    FuzzerViewMainCallback callback;
    void* context;
};

typedef struct {
    uint8_t proto_index;
    uint8_t menu_index;
    uint8_t proto_max;
    uint8_t menu_max;
} FuzzerViewMainModel;

void fuzzer_view_main_update_data(FuzzerViewMain* view, FuzzerState state) {
    furi_assert(view);
    with_view_model(
        view->view,
        FuzzerViewMainModel * model,
        {
            model->proto_index = state.proto_index;
            model->menu_index = state.menu_index;
        },
        true);
}

void fuzzer_view_main_get_state(FuzzerViewMain* view, FuzzerState* state) {
    furi_assert(view);
    with_view_model(
        view->view,
        FuzzerViewMainModel * model,
        {
            state->proto_index = model->proto_index;
            state->menu_index = model->menu_index;
        },
        true);
}

void fuzzer_view_main_set_callback(
    FuzzerViewMain* view,
    FuzzerViewMainCallback callback,
    void* context) {
    furi_assert(view);

    view->callback = callback;
    view->context = context;
}

void fuzzer_view_main_draw(Canvas* canvas, FuzzerViewMainModel* model) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    if(model->menu_index > 0) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(
            canvas,
            64,
            24,
            AlignCenter,
            AlignTop,
            fuzzer_proto_get_menu_label(model->menu_index - 1));
    }

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas, 64, 36, AlignCenter, AlignTop, fuzzer_proto_get_menu_label(model->menu_index));

    if(model->menu_index < (model->menu_max - 1)) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(
            canvas,
            64,
            48,
            AlignCenter,
            AlignTop,
            fuzzer_proto_get_menu_label(model->menu_index + 1));
    }

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 27, PROTOCOL_NAME_Y, AlignCenter, AlignBottom, "<");
    canvas_draw_str_aligned(
        canvas,
        64,
        PROTOCOL_NAME_Y,
        AlignCenter,
        AlignBottom,
        fuzzer_proto_get_name(model->proto_index));
    canvas_draw_str_aligned(canvas, 101, PROTOCOL_NAME_Y, AlignCenter, AlignBottom, ">");

#ifdef PROTOCOL_CAROUSEL
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(
        canvas,
        20,
        PROTOCOL_NAME_Y,
        AlignRight,
        AlignBottom,
        (model->proto_index > 0) ? fuzzer_proto_get_name(model->proto_index - 1) :
                                   fuzzer_proto_get_name((model->proto_max - 1)));
    canvas_draw_str_aligned(
        canvas,
        108,
        PROTOCOL_NAME_Y,
        AlignLeft,
        AlignBottom,
        (model->proto_index < (model->proto_max - 1)) ?
            fuzzer_proto_get_name(model->proto_index + 1) :
            fuzzer_proto_get_name(0));
#endif
}

bool fuzzer_view_main_input(InputEvent* event, void* context) {
    furi_assert(context);
    FuzzerViewMain* view = context;

    if(event->key == InputKeyBack &&
       (event->type == InputTypeLong || event->type == InputTypeShort)) {
        view->callback(FuzzerCustomEventViewMainBack, view->context);
        return true;
    } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
        view->callback(FuzzerCustomEventViewMainOk, view->context);
        return true;
    } else if(event->key == InputKeyDown && event->type == InputTypeShort) {
        with_view_model(
            view->view,
            FuzzerViewMainModel * model,
            {
                if(model->menu_index < (model->menu_max - 1)) {
                    model->menu_index++;
                }
            },
            true);
        return true;
    } else if(event->key == InputKeyUp && event->type == InputTypeShort) {
        with_view_model(
            view->view,
            FuzzerViewMainModel * model,
            {
                if(model->menu_index != 0) {
                    model->menu_index--;
                }
            },
            true);
        return true;
    } else if(event->key == InputKeyLeft && event->type == InputTypeShort) {
        with_view_model(
            view->view,
            FuzzerViewMainModel * model,
            {
                if(model->proto_index != 0) {
                    model->proto_index--;
                } else {
                    model->proto_index = (model->proto_max - 1);
                }
            },
            true);
        return true;
    } else if(event->key == InputKeyRight && event->type == InputTypeShort) {
        with_view_model(
            view->view,
            FuzzerViewMainModel * model,
            {
                if(model->proto_index == (model->proto_max - 1)) {
                    model->proto_index = 0;
                } else {
                    model->proto_index++;
                }
            },
            true);
        return true;
    }

    return true;
}

void fuzzer_view_main_enter(void* context) {
    furi_assert(context);
}

void fuzzer_view_main_exit(void* context) {
    furi_assert(context);
}

FuzzerViewMain* fuzzer_view_main_alloc() {
    FuzzerViewMain* view = malloc(sizeof(FuzzerViewMain));

    // View allocation and configuration
    view->view = view_alloc();
    view_allocate_model(view->view, ViewModelTypeLocking, sizeof(FuzzerViewMainModel));
    view_set_context(view->view, view);
    view_set_draw_callback(view->view, (ViewDrawCallback)fuzzer_view_main_draw);
    view_set_input_callback(view->view, fuzzer_view_main_input);
    view_set_enter_callback(view->view, fuzzer_view_main_enter);
    view_set_exit_callback(view->view, fuzzer_view_main_exit);

    with_view_model(
        view->view,
        FuzzerViewMainModel * model,
        {
            model->proto_index = 0;
            model->proto_max = fuzzer_proto_get_count_of_protocols();
            model->menu_index = 0;
            model->menu_max = fuzzer_proto_get_count_of_menu_items();
        },
        true);
    return view;
}

void fuzzer_view_main_free(FuzzerViewMain* view) {
    furi_assert(view);

    // with_view_model(
    //     view->view,
    //     FuzzerViewMainModel * model,
    //     {

    //     },
    //     true);
    view_free(view->view);
    free(view);
}

View* fuzzer_view_main_get_view(FuzzerViewMain* view) {
    furi_assert(view);
    return view->view;
}