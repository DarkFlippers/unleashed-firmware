#include "avr_isp_view_programmer.h"
#include <avr_isp_icons.h>

#include "../helpers/avr_isp_worker.h"
#include <gui/elements.h>

struct AvrIspProgrammerView {
    View* view;
    AvrIspWorker* worker;
    AvrIspProgrammerViewCallback callback;
    void* context;
};

typedef struct {
    AvrIspProgrammerViewStatus status;
} AvrIspProgrammerViewModel;

void avr_isp_programmer_view_set_callback(
    AvrIspProgrammerView* instance,
    AvrIspProgrammerViewCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

void avr_isp_programmer_view_draw(Canvas* canvas, AvrIspProgrammerViewModel* model) {
    canvas_clear(canvas);

    if(model->status == AvrIspProgrammerViewStatusUSBConnect) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_icon(canvas, 0, 0, &I_isp_active_128x53);
        elements_multiline_text(canvas, 45, 10, "ISP mode active");
    } else {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_icon(canvas, 51, 6, &I_link_waiting_77x56);
        elements_multiline_text(canvas, 0, 25, "Waiting for\nsoftware\nconnection");
    }
}

bool avr_isp_programmer_view_input(InputEvent* event, void* context) {
    furi_assert(context);
    UNUSED(context);

    if(event->key == InputKeyBack || event->type != InputTypeShort) {
        return false;
    }

    return true;
}

static void avr_isp_programmer_usb_connect_callback(void* context, bool status_connect) {
    furi_assert(context);
    AvrIspProgrammerView* instance = context;

    with_view_model(
        instance->view,
        AvrIspProgrammerViewModel * model,
        {
            if(status_connect) {
                model->status = AvrIspProgrammerViewStatusUSBConnect;
            } else {
                model->status = AvrIspProgrammerViewStatusNoUSBConnect;
            }
        },
        true);
}

void avr_isp_programmer_view_enter(void* context) {
    furi_assert(context);

    AvrIspProgrammerView* instance = context;
    with_view_model(
        instance->view,
        AvrIspProgrammerViewModel * model,
        { model->status = AvrIspProgrammerViewStatusNoUSBConnect; },
        true);

    //Start worker
    instance->worker = avr_isp_worker_alloc(instance->context);

    avr_isp_worker_set_callback(
        instance->worker, avr_isp_programmer_usb_connect_callback, instance);

    avr_isp_worker_start(instance->worker);
}

void avr_isp_programmer_view_exit(void* context) {
    furi_assert(context);

    AvrIspProgrammerView* instance = context;
    //Stop worker
    if(avr_isp_worker_is_running(instance->worker)) {
        avr_isp_worker_stop(instance->worker);
    }
    avr_isp_worker_set_callback(instance->worker, NULL, NULL);
    avr_isp_worker_free(instance->worker);
}

AvrIspProgrammerView* avr_isp_programmer_view_alloc() {
    AvrIspProgrammerView* instance = malloc(sizeof(AvrIspProgrammerView));

    // View allocation and configuration
    instance->view = view_alloc();

    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(AvrIspProgrammerViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)avr_isp_programmer_view_draw);
    view_set_input_callback(instance->view, avr_isp_programmer_view_input);
    view_set_enter_callback(instance->view, avr_isp_programmer_view_enter);
    view_set_exit_callback(instance->view, avr_isp_programmer_view_exit);

    with_view_model(
        instance->view,
        AvrIspProgrammerViewModel * model,
        { model->status = AvrIspProgrammerViewStatusNoUSBConnect; },
        false);
    return instance;
}

void avr_isp_programmer_view_free(AvrIspProgrammerView* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* avr_isp_programmer_view_get_view(AvrIspProgrammerView* instance) {
    furi_assert(instance);

    return instance->view;
}
