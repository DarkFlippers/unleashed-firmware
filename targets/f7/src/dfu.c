#include <furi.h>
#include <furi_hal.h>
#include <flipper.h>
#include <alt_boot.h>
#include <assets_icons.h>
#include <toolbox/compress.h>
#include <gui/canvas.h>
#include <gui/canvas_i.h>

void flipper_boot_dfu_show_splash(void) {
    // Initialize
    Canvas* canvas = canvas_init();

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);

    canvas_draw_icon(canvas, 0, 64 - 50, &I_DFU_128x50);
    canvas_draw_str(canvas, 2, 8, "Update & Recovery Mode");
    canvas_draw_str(canvas, 2, 21, "DFU Started");
    canvas_commit(canvas);

    canvas_free(canvas);
}

void flipper_boot_dfu_exec(void) {
    // Show DFU splashscreen
    flipper_boot_dfu_show_splash();

    // Errata 2.2.9, Flash OPTVERR flag is always set after system reset
    WRITE_REG(FLASH->SR, FLASH_SR_OPTVERR);

    // Cleanup before jumping to DFU
    furi_hal_deinit_early();

    // Remap memory to system bootloader
    LL_SYSCFG_SetRemapMemory(LL_SYSCFG_REMAP_SYSTEMFLASH);
    // Jump
    furi_hal_switch(0x0);
}
