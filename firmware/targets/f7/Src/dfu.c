#include <furi.h>
#include <furi_hal.h>
#include <flipper.h>
#include <alt_boot.h>
#include <u8g2_glue.h>
#include <assets_icons.h>

void flipper_boot_dfu_show_splash() {
    // Initialize
    furi_hal_compress_icon_init();

    u8g2_t* fb = malloc(sizeof(u8g2_t));
    memset(fb, 0, sizeof(u8g2_t));
    u8g2_Setup_st756x_flipper(fb, U8G2_R0, u8x8_hw_spi_stm32, u8g2_gpio_and_delay_stm32);
    u8g2_InitDisplay(fb);
    u8g2_SetDrawColor(fb, 0x01);
    uint8_t* splash_data = NULL;
    furi_hal_compress_icon_decode(icon_get_data(&I_DFU_128x50), &splash_data);
    u8g2_DrawXBM(fb, 0, 64 - 50, 128, 50, splash_data);
    u8g2_SetFont(fb, u8g2_font_helvB08_tr);
    u8g2_DrawStr(fb, 2, 8, "Update & Recovery Mode");
    u8g2_DrawStr(fb, 2, 21, "DFU Started");
    u8g2_SetPowerSave(fb, 0);
    u8g2_SendBuffer(fb);
}

void flipper_boot_dfu_exec() {
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
