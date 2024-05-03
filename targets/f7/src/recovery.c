#include <furi.h>
#include <furi_hal.h>
#include <flipper.h>
#include <alt_boot.h>
#include <assets_icons.h>
#include <toolbox/compress.h>
#include <gui/canvas.h>
#include <gui/canvas_i.h>

#define COUNTER_VALUE (136U)

static void flipper_boot_recovery_draw_progress(Canvas* canvas, size_t progress) {
    if(progress < COUNTER_VALUE) {
        // Fill the progress bar while the progress is going down
        canvas_draw_rframe(canvas, 59, 41, 69, 8, 2);
        size_t width = (COUNTER_VALUE - progress) * 68 / COUNTER_VALUE;
        canvas_draw_box(canvas, 60, 42, width, 6);
    } else {
        canvas_draw_rframe(canvas, 59, 41, 69, 8, 2);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, 60, 42, 67, 6);
        canvas_set_color(canvas, ColorBlack);
    }

    canvas_commit(canvas);
}

void flipper_boot_recovery_draw_splash(Canvas* canvas) {
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);

    canvas_draw_icon(canvas, 0, 0, &I_Erase_pin_128x64);

    canvas_commit(canvas);
}

void flipper_boot_recovery_exec(void) {
    Canvas* canvas = canvas_init();

    // Show recovery splashscreen
    flipper_boot_recovery_draw_splash(canvas);

    size_t counter = COUNTER_VALUE;
    while(counter) {
        if(!furi_hal_gpio_read(&gpio_button_down)) {
            break;
        }

        if(!furi_hal_gpio_read(&gpio_button_right)) {
            counter--;
        } else {
            counter = COUNTER_VALUE;
        }

        flipper_boot_recovery_draw_progress(canvas, counter);
    }

    if(!counter) {
        furi_hal_rtc_reset_registers();
        furi_hal_rtc_set_flag(FuriHalRtcFlagStorageFormatInternal);
    }
}
