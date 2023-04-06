#include "avr_isp_view_writer.h"
#include <gui/elements.h>

#include "../helpers/avr_isp_worker_rw.h"
#include <float_tools.h>

struct AvrIspWriterView {
    View* view;
    AvrIspWorkerRW* avr_isp_worker_rw;
    const char* file_path;
    const char* file_name;
    AvrIspWriterViewCallback callback;
    void* context;
};

typedef struct {
    AvrIspWriterViewStatus status;
    float progress_flash;
    float progress_eeprom;
} AvrIspWriterViewModel;

void avr_isp_writer_update_progress(AvrIspWriterView* instance) {
    with_view_model(
        instance->view,
        AvrIspWriterViewModel * model,
        {
            model->progress_flash =
                avr_isp_worker_rw_get_progress_flash(instance->avr_isp_worker_rw);
            model->progress_eeprom =
                avr_isp_worker_rw_get_progress_eeprom(instance->avr_isp_worker_rw);
        },
        true);
}

void avr_isp_writer_view_set_callback(
    AvrIspWriterView* instance,
    AvrIspWriterViewCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

void avr_isp_writer_set_file_path(
    AvrIspWriterView* instance,
    const char* file_path,
    const char* file_name) {
    furi_assert(instance);

    instance->file_path = file_path;
    instance->file_name = file_name;
}

void avr_isp_writer_view_draw(Canvas* canvas, AvrIspWriterViewModel* model) {
    canvas_clear(canvas);
    char str_flash[32] = {0};
    char str_eeprom[32] = {0};

    canvas_set_font(canvas, FontPrimary);

    switch(model->status) {
    case AvrIspWriterViewStatusIDLE:
        canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignCenter, "Press start to write");
        canvas_set_font(canvas, FontSecondary);
        elements_button_center(canvas, "Start");
        snprintf(str_flash, sizeof(str_flash), "%d%%", (uint8_t)(model->progress_flash * 100));
        snprintf(str_eeprom, sizeof(str_eeprom), "%d%%", (uint8_t)(model->progress_eeprom * 100));
        break;
    case AvrIspWriterViewStatusWriting:
        if(float_is_equal(model->progress_flash, 0.f)) {
            canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignCenter, "Verifying firmware");
            snprintf(str_flash, sizeof(str_flash), "***");
            snprintf(str_eeprom, sizeof(str_eeprom), "***");
        } else {
            canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignCenter, "Writing dump");
            snprintf(str_flash, sizeof(str_flash), "%d%%", (uint8_t)(model->progress_flash * 100));
            snprintf(
                str_eeprom, sizeof(str_eeprom), "%d%%", (uint8_t)(model->progress_eeprom * 100));
        }
        break;
    case AvrIspWriterViewStatusVerification:
        canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignCenter, "Verifying dump");
        snprintf(str_flash, sizeof(str_flash), "%d%%", (uint8_t)(model->progress_flash * 100));
        snprintf(str_eeprom, sizeof(str_eeprom), "%d%%", (uint8_t)(model->progress_eeprom * 100));
        break;
    case AvrIspWriterViewStatusWritingFuse:
        canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignCenter, "Writing fuse");
        snprintf(str_flash, sizeof(str_flash), "%d%%", (uint8_t)(model->progress_flash * 100));
        snprintf(str_eeprom, sizeof(str_eeprom), "%d%%", (uint8_t)(model->progress_eeprom * 100));
        break;
    case AvrIspWriterViewStatusWritingFuseOk:
        canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignCenter, "Done!");
        snprintf(str_flash, sizeof(str_flash), "%d%%", (uint8_t)(model->progress_flash * 100));
        snprintf(str_eeprom, sizeof(str_eeprom), "%d%%", (uint8_t)(model->progress_eeprom * 100));
        canvas_set_font(canvas, FontSecondary);
        elements_button_center(canvas, "Reflash");
        elements_button_right(canvas, "Exit");
        break;

    default:
        break;
    }

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 27, "Flash");
    // snprintf(str_buf, sizeof(str_buf), "%d%%", (uint8_t)(model->progress_flash * 100));
    elements_progress_bar_with_text(canvas, 44, 17, 84, model->progress_flash, str_flash);
    canvas_draw_str(canvas, 0, 43, "EEPROM");
    // snprintf(str_buf, sizeof(str_buf), "%d%%", (uint8_t)(model->progress_eeprom * 100));
    elements_progress_bar_with_text(canvas, 44, 34, 84, model->progress_eeprom, str_eeprom);
}

bool avr_isp_writer_view_input(InputEvent* event, void* context) {
    furi_assert(context);
    AvrIspWriterView* instance = context;

    bool ret = true;
    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        with_view_model(
            instance->view,
            AvrIspWriterViewModel * model,
            {
                if((model->status == AvrIspWriterViewStatusIDLE) ||
                   (model->status == AvrIspWriterViewStatusWritingFuseOk)) {
                    if(instance->callback)
                        instance->callback(AvrIspCustomEventSceneExit, instance->context);
                    ret = false;
                }
            },
            false);
    } else if(event->key == InputKeyOk && event->type == InputTypeShort) {
        with_view_model(
            instance->view,
            AvrIspWriterViewModel * model,
            {
                if((model->status == AvrIspWriterViewStatusIDLE) ||
                   (model->status == AvrIspWriterViewStatusWritingFuseOk)) {
                    model->status = AvrIspWriterViewStatusWriting;

                    avr_isp_worker_rw_write_dump_start(
                        instance->avr_isp_worker_rw, instance->file_path, instance->file_name);
                }
            },
            false);
    } else if(event->key == InputKeyRight && event->type == InputTypeShort) {
        with_view_model(
            instance->view,
            AvrIspWriterViewModel * model,
            {
                if((model->status == AvrIspWriterViewStatusIDLE) ||
                   (model->status == AvrIspWriterViewStatusWritingFuseOk)) {
                    if(instance->callback)
                        instance->callback(AvrIspCustomEventSceneExitStartMenu, instance->context);
                    ret = false;
                }
            },
            false);
    }
    return ret;
}

static void avr_isp_writer_callback_status(void* context, AvrIspWorkerRWStatus status) {
    furi_assert(context);

    AvrIspWriterView* instance = context;
    with_view_model(
        instance->view,
        AvrIspWriterViewModel * model,
        {
            switch(status) {
            case AvrIspWorkerRWStatusEndWriting:
                model->status = AvrIspWriterViewStatusVerification;
                avr_isp_worker_rw_verification_start(
                    instance->avr_isp_worker_rw, instance->file_path, instance->file_name);
                model->status = AvrIspWriterViewStatusVerification;
                break;
            case AvrIspWorkerRWStatusErrorVerification:
                if(instance->callback)
                    instance->callback(AvrIspCustomEventSceneErrorVerification, instance->context);
                break;
            case AvrIspWorkerRWStatusEndVerification:
                avr_isp_worker_rw_write_fuse_start(
                    instance->avr_isp_worker_rw, instance->file_path, instance->file_name);
                model->status = AvrIspWriterViewStatusWritingFuse;
                break;
            case AvrIspWorkerRWStatusErrorWritingFuse:
                if(instance->callback)
                    instance->callback(AvrIspCustomEventSceneErrorWritingFuse, instance->context);
                break;
            case AvrIspWorkerRWStatusEndWritingFuse:
                model->status = AvrIspWriterViewStatusWritingFuseOk;
                break;

            default:
                //AvrIspWorkerRWStatusErrorWriting;
                if(instance->callback)
                    instance->callback(AvrIspCustomEventSceneErrorWriting, instance->context);
                break;
            }
        },
        true);
}

void avr_isp_writer_view_enter(void* context) {
    furi_assert(context);

    AvrIspWriterView* instance = context;
    with_view_model(
        instance->view,
        AvrIspWriterViewModel * model,
        {
            model->status = AvrIspWriterViewStatusIDLE;
            model->progress_flash = 0.0f;
            model->progress_eeprom = 0.0f;
        },
        true);

    //Start avr_isp_worker_rw
    instance->avr_isp_worker_rw = avr_isp_worker_rw_alloc(instance->context);

    avr_isp_worker_rw_set_callback_status(
        instance->avr_isp_worker_rw, avr_isp_writer_callback_status, instance);

    avr_isp_worker_rw_start(instance->avr_isp_worker_rw);
}

void avr_isp_writer_view_exit(void* context) {
    furi_assert(context);
    AvrIspWriterView* instance = context;

    //Stop avr_isp_worker_rw
    if(avr_isp_worker_rw_is_running(instance->avr_isp_worker_rw)) {
        avr_isp_worker_rw_stop(instance->avr_isp_worker_rw);
    }

    avr_isp_worker_rw_free(instance->avr_isp_worker_rw);
}

AvrIspWriterView* avr_isp_writer_view_alloc() {
    AvrIspWriterView* instance = malloc(sizeof(AvrIspWriterView));

    // View allocation and configuration
    instance->view = view_alloc();

    view_allocate_model(instance->view, ViewModelTypeLocking, sizeof(AvrIspWriterViewModel));
    view_set_context(instance->view, instance);
    view_set_draw_callback(instance->view, (ViewDrawCallback)avr_isp_writer_view_draw);
    view_set_input_callback(instance->view, avr_isp_writer_view_input);
    view_set_enter_callback(instance->view, avr_isp_writer_view_enter);
    view_set_exit_callback(instance->view, avr_isp_writer_view_exit);

    return instance;
}

void avr_isp_writer_view_free(AvrIspWriterView* instance) {
    furi_assert(instance);

    view_free(instance->view);
    free(instance);
}

View* avr_isp_writer_view_get_view(AvrIspWriterView* instance) {
    furi_assert(instance);

    return instance->view;
}
