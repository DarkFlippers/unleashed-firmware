#include "main_menu.h"
#include "../fuzzer_i.h"

#include <input/input.h>
#include <gui/elements.h>

struct FuzzerViewMain {
    View* view;
    FuzzerViewMainCallback callback;
    void* context;
};

typedef struct {
    uint8_t proto_index;
    uint8_t menu_index;
} FuzzerViewMainModel;

void fuzzer_view_main_set_callback(
    FuzzerViewMain* fuzzer_view_main,
    FuzzerViewMainCallback callback,
    void* context) {
    furi_assert(fuzzer_view_main);

    fuzzer_view_main->callback = callback;
    fuzzer_view_main->context = context;
}

void fuzzer_view_main_draw(Canvas* canvas, FuzzerViewMainModel* model) {
    UNUSED(canvas);
    UNUSED(model);
}

bool fuzzer_view_main_input(InputEvent* event, void* context) {
    furi_assert(context);
    FuzzerViewMain* fuzzer_view_main = context;

    if(event->key == InputKeyBack &&
       (event->type == InputTypeLong || event->type == InputTypeShort)) {
        fuzzer_view_main->callback(FuzzerCustomEventViewMainBack, fuzzer_view_main->context);
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
    FuzzerViewMain* fuzzer_view_main = malloc(sizeof(FuzzerViewMain));

    // View allocation and configuration
    fuzzer_view_main->view = view_alloc();
    view_allocate_model(fuzzer_view_main->view, ViewModelTypeLocking, sizeof(FuzzerViewMainModel));
    view_set_context(fuzzer_view_main->view, fuzzer_view_main);
    view_set_draw_callback(fuzzer_view_main->view, (ViewDrawCallback)fuzzer_view_main_draw);
    view_set_input_callback(fuzzer_view_main->view, fuzzer_view_main_input);
    view_set_enter_callback(fuzzer_view_main->view, fuzzer_view_main_enter);
    view_set_exit_callback(fuzzer_view_main->view, fuzzer_view_main_exit);

    with_view_model(
        fuzzer_view_main->view,
        FuzzerViewMainModel * model,
        {
            model->proto_index = 0;
            model->menu_index = 0;
        },
        true);
    return fuzzer_view_main;
}

void fuzzer_view_main_free(FuzzerViewMain* fuzzer_view_main) {
    furi_assert(fuzzer_view_main);

    // with_view_model(
    //     fuzzer_view_main->view,
    //     FuzzerViewMainModel * model,
    //     {

    //     },
    //     true);
    view_free(fuzzer_view_main->view);
    free(fuzzer_view_main);
}

View* fuzzer_view_main_get_view(FuzzerViewMain* fuzzer_view_main) {
    furi_assert(fuzzer_view_main);
    return fuzzer_view_main->view;
}