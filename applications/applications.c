#include "applications.h"

#ifdef APP_TEST
void flipper_test_app(void* p);
#endif

void application_blink(void* p);
void application_uart_write(void* p);
void application_ipc_display(void* p);
void application_ipc_widget(void* p);
void application_input_dump(void* p);
void display_u8g2(void* p);
void u8g2_example(void* p);
void input_task(void* p);
void menu_task(void* p);
void coreglitch_demo_0(void* p);
void u8g2_qrcode(void* p);
void fatfs_list(void* p);
void gui_task(void* p);
void backlight_control(void* p);
void irda(void* p);
void app_loader(void* p);
void cc1101_workaround(void* p);
void lf_rfid_workaround(void* p);
void nfc_task(void* p);
void dolphin_task(void* p);
void power_task(void* p);
void bt_task(void* p);
void sd_card_test(void* p);
void application_vibro(void* p);
void app_gpio_test(void* p);
void app_ibutton(void* p);
void cli_task(void* p);
void music_player(void* p);
void sdnfc(void* p);
void floopper_bloopper(void* p);
void sd_filesystem(void* p);

const FuriApplication FLIPPER_SERVICES[] = {
#ifdef APP_DISPLAY
    {.app = display_u8g2, .name = "display_u8g2", .icon = A_Plugins_14},
#endif

#ifdef APP_CLI
    {.app = cli_task, .name = "cli_task", .icon = A_Plugins_14},
#endif

#ifdef APP_EXAMPLE_BLINK
    {.app = application_blink, .name = "blink", .icon = A_Plugins_14},
#endif

#ifdef APP_INPUT
    {.app = input_task, .name = "input_task", .icon = A_Plugins_14},
#endif

#ifdef APP_EXAMPLE_INPUT_DUMP
    {.app = application_input_dump, .name = "input dump", .icon = A_Plugins_14},
#endif

#ifdef APP_GUI
    {.app = backlight_control, .name = "backlight_control", .icon = A_Plugins_14},
    {.app = gui_task, .name = "gui_task", .icon = A_Plugins_14},
#endif

#ifdef APP_MENU
    {.app = menu_task, .name = "menu_task", .icon = A_Plugins_14},
    {.app = app_loader, .name = "app_loader", .icon = A_Plugins_14},
#endif

#ifdef APP_SD_FILESYSTEM
    {.app = sd_filesystem, .name = "sd_filesystem", .icon = A_Plugins_14},
#endif

#ifdef APP_DOLPHIN
    {.app = dolphin_task, .name = "dolphin_task", .icon = A_Plugins_14},
#endif

#ifdef APP_POWER
    {.app = power_task, .name = "power_task", .icon = A_Plugins_14},
#endif

#ifdef APP_BT
    {.app = bt_task, .name = "bt_task", .icon = A_Plugins_14},
#endif

#ifdef APP_CC1101
    {.app = cc1101_workaround, .name = "cc1101 workaround", .icon = A_Plugins_14},
#endif

#ifdef APP_LF_RFID
    {.app = lf_rfid_workaround, .name = "lf rfid workaround", .icon = A_Plugins_14},
#endif

#ifdef APP_IRDA
    {.app = irda, .name = "irda", .icon = A_Plugins_14},
#endif

#ifdef APP_NFC
    {.app = nfc_task, .name = "nfc_task", .icon = A_Plugins_14},
#endif

#ifdef APP_TEST
    {.app = flipper_test_app, .name = "test app", .icon = A_Plugins_14},
#endif

#ifdef APP_EXAMPLE_IPC
    {.app = application_ipc_display, .name = "ipc display", .icon = A_Plugins_14},
    {.app = application_ipc_widget, .name = "ipc widget", .icon = A_Plugins_14},
#endif

#ifdef APP_EXAMPLE_QRCODE
    {.app = u8g2_qrcode, .name = "u8g2_qrcode", .icon = A_Plugins_14},
#endif

#ifdef APP_EXAMPLE_FATFS
    {.app = fatfs_list, .name = "fatfs_list", .icon = A_Plugins_14},
#endif

#ifdef APP_EXAMPLE_DISPLAY
    {.app = u8g2_example, .name = "u8g2_example", .icon = A_Plugins_14},
#endif

#ifdef APP_SPEAKER_DEMO
    {.app = coreglitch_demo_0, .name = "coreglitch_demo_0", .icon = A_Plugins_14},
#endif

#ifdef APP_SD_TEST
    {.app = sd_card_test, .name = "sd_card_test", .icon = A_Plugins_14},
#endif

#ifdef APP_MUSIC_PLAYER
    {.app = music_player, .name = "music player", .icon = A_Plugins_14},
#endif

#ifdef APP_IBUTTON
    {.app = app_ibutton, .name = "ibutton", .icon = A_Plugins_14},
#endif

#ifdef APP_GPIO_DEMO
    {.app = app_gpio_test, .name = "gpio test", .icon = A_Plugins_14},
#endif

#ifdef APP_FLOOPPER_BLOOPPER
    {.app = floopper_bloopper, .name = "Floopper Bloopper", .icon = A_Games_14},
#endif

#ifdef APP_SDNFC
    {.app = sdnfc, .name = "sdnfc", .icon = A_Plugins_14},
#endif
};

size_t FLIPPER_SERVICES_size() {
    return sizeof(FLIPPER_SERVICES) / sizeof(FuriApplication);
}

// Main menu APP
const FuriApplication FLIPPER_APPS[] = {
#ifdef BUILD_CC1101
    {.app = cc1101_workaround, .name = "Sub-1 GHz", .icon = A_Sub1ghz_14},
#endif

#ifdef BUILD_LF_RFID
    {.app = lf_rfid_workaround, .name = "125 kHz RFID", .icon = A_125khz_14},
#endif

#ifdef BUILD_IRDA
    {.app = irda, .name = "Infrared", .icon = A_Infrared_14},
#endif

#ifdef BUILD_IBUTTON
    {.app = app_ibutton, .name = "iButton", .icon = A_iButton_14},
#endif

#ifdef BUILD_GPIO_DEMO
    {.app = app_gpio_test, .name = "GPIO", .icon = A_GPIO_14},
#endif
};

size_t FLIPPER_APPS_size() {
    return sizeof(FLIPPER_APPS) / sizeof(FuriApplication);
}

// Plugin menu
const FuriApplication FLIPPER_PLUGINS[] = {
#ifdef BUILD_EXAMPLE_BLINK
    {.app = application_blink, .name = "blink", .icon = A_Plugins_14},
#endif

#ifdef BUILD_EXAMPLE_INPUT_DUMP
    {.app = application_input_dump, .name = "input dump", .icon = A_Plugins_14},
#endif

#ifdef BUILD_SPEAKER_DEMO
    {.app = coreglitch_demo_0, .name = "coreglitch_demo_0", .icon = A_Plugins_14},
#endif

#ifdef BUILD_SD_TEST
    {.app = sd_card_test, .name = "sd_card_test", .icon = A_Plugins_14},
#endif

#ifdef BUILD_VIBRO_DEMO
    {.app = application_vibro, .name = "application_vibro", .icon = A_Plugins_14},
#endif

#ifdef BUILD_MUSIC_PLAYER
    {.app = music_player, .name = "music player", .icon = A_Plugins_14},
#endif

#ifdef BUILD_FLOOPPER_BLOOPPER
    {.app = floopper_bloopper, .name = "Floopper Bloopper", .icon = A_Games_14},
#endif

#ifdef BUILD_SDNFC
    {.app = sdnfc, .name = "sdnfc", .icon = A_Plugins_14},
#endif
};

size_t FLIPPER_PLUGINS_size() {
    return sizeof(FLIPPER_PLUGINS) / sizeof(FuriApplication);
}
