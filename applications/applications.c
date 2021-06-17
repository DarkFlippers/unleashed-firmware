#include "applications.h"

// Services and apps decalartion
int32_t application_vertical_screen(void* p);
int32_t irda_monitor_app(void* p);
int32_t flipper_test_app(void* p);
int32_t application_blink(void* p);
int32_t application_uart_write(void* p);
int32_t application_input_dump(void* p);
int32_t u8g2_example(void* p);
int32_t input_task(void* p);
int32_t menu_task(void* p);
int32_t coreglitch_demo_0(void* p);
int32_t u8g2_qrcode(void* p);
int32_t gui_task(void* p);
int32_t backlight_control(void* p);
int32_t irda(void* p);
int32_t app_loader(void* p);
int32_t app_lfrfid(void* p);
int32_t nfc_task(void* p);
int32_t dolphin_task(void* p);
int32_t power_task(void* p);
int32_t bt_task(void* p);
int32_t sd_card_test(void* p);
int32_t application_vibro(void* p);
int32_t app_gpio_test(void* p);
int32_t app_ibutton(void* p);
int32_t cli_task(void* p);
int32_t music_player(void* p);
int32_t sdnfc(void* p);
int32_t floopper_bloopper(void* p);
int32_t sd_filesystem(void* p);
int32_t subghz_app(void* p);
int32_t gui_test(void* p);
int32_t keypad_test(void* p);
int32_t scene_app(void* p);
int32_t passport(void* p);
int32_t app_accessor(void* p);
int32_t internal_storage_task(void* p);
int32_t app_archive(void* p);
int32_t notification_app(void* p);
int32_t scened_app(void* p);

// On system start hooks declaration
void irda_cli_init();
void nfc_cli_init();
void subghz_cli_init();
void bt_cli_init();
void lfrfid_cli_init();
void ibutton_cli_init();

const FlipperApplication FLIPPER_SERVICES[] = {
#ifdef SRV_CLI
    {.app = cli_task, .name = "cli_task", .stack_size = 2048, .icon = A_Plugins_14},
#endif

#ifdef SRV_EXAMPLE_BLINK
    {.app = application_blink, .name = "blink", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef SRV_INPUT
    {.app = input_task, .name = "input_task", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef SRV_EXAMPLE_INPUT_DUMP
    {.app = application_input_dump, .name = "input dump", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef SRV_GUI
    {.app = backlight_control,
     .name = "backlight_control",
     .stack_size = 1024,
     .icon = A_Plugins_14},
    // TODO: fix stack size when sd api will be in separate thread
    {.app = gui_task, .name = "gui_task", .stack_size = 8192, .icon = A_Plugins_14},
#endif

#ifdef SRV_MENU
    {.app = menu_task, .name = "menu_task", .stack_size = 1024, .icon = A_Plugins_14},
    {.app = app_loader, .name = "app_loader", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef SRV_SD_FILESYSTEM
    {.app = sd_filesystem, .name = "sd_filesystem", .stack_size = 4096, .icon = A_Plugins_14},
#endif

#ifdef SRV_INTERNAL_STORAGE
    {.app = internal_storage_task,
     .name = "internal_storage",
     .stack_size = 2048,
     .icon = A_Plugins_14},
#endif

#ifdef SRV_DOLPHIN
    {.app = dolphin_task, .name = "dolphin_task", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef SRV_POWER
    {.app = power_task, .name = "power_task", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef SRV_BT
    {.app = bt_task, .name = "bt_task", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef SRV_LF_RFID
    {.app = app_lfrfid, .name = "125 kHz RFID", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef SRV_IRDA
    {.app = irda, .name = "irda", .stack_size = 1024 * 3, .icon = A_Plugins_14},
#endif

#ifdef SRV_EXAMPLE_QRCODE
    {.app = u8g2_qrcode, .name = "u8g2_qrcode", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef SRV_EXAMPLE_DISPLAY
    {.app = u8g2_example, .name = "u8g2_example", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef SRV_SPEAKER_DEMO
    {.app = coreglitch_demo_0,
     .name = "coreglitch_demo_0",
     .stack_size = 1024,
     .icon = A_Plugins_14},
#endif

#ifdef SRV_SD_TEST
    {.app = sd_card_test, .name = "sd_card_test", .stack_size = 4096, .icon = A_Plugins_14},
#endif

#ifdef SRV_MUSIC_PLAYER
    {.app = music_player, .name = "music player", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef SRV_IBUTTON
    {.app = app_ibutton, .name = "ibutton", .stack_size = 4096, .icon = A_Plugins_14},
#endif

#ifdef SRV_GPIO_DEMO
    {.app = app_gpio_test, .name = "gpio test", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef SRV_FLOOPPER_BLOOPPER
    {.app = floopper_bloopper, .name = "Floopper Bloopper", .stack_size = 1024, .icon = A_Games_14},
#endif

#ifdef SRV_SDNFC
    {.app = sdnfc, .name = "sdnfc", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef SRV_GUI_TEST
    {.app = gui_test, .name = "gui_test", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef SRV_KEYPAD_TEST
    {.app = keypad_test, .name = "keypad_test", .icon = A_Plugins_14},
#endif

#ifdef SRV_ACCESSOR
    {.app = app_accessor, .name = "accessor", .stack_size = 4096, .icon = A_Plugins_14},
#endif

#ifdef SRV_NOTIFICATION
    {.app = notification_app, .name = "notification", .stack_size = 1024, .icon = A_Plugins_14},
#endif

};

const size_t FLIPPER_SERVICES_COUNT = sizeof(FLIPPER_SERVICES) / sizeof(FlipperApplication);

// Main menu APP
const FlipperApplication FLIPPER_APPS[] = {

#ifdef APP_IBUTTON
    {.app = app_ibutton, .name = "iButton", .stack_size = 4096, .icon = A_iButton_14},
#endif

#ifdef APP_NFC
    {.app = nfc_task, .name = "NFC", .stack_size = 1024, .icon = A_NFC_14},
#endif

#ifdef APP_SUBGHZ
    {.app = subghz_app, .name = "Sub-1 GHz", .stack_size = 1024, .icon = A_Sub1ghz_14},
#endif

#ifdef APP_LF_RFID
    {.app = app_lfrfid, .name = "125 kHz RFID", .stack_size = 1024, .icon = A_125khz_14},
#endif

#ifdef APP_IRDA
    {.app = irda, .name = "Infrared", .stack_size = 1024 * 3, .icon = A_Infrared_14},
#endif

#ifdef APP_GPIO_DEMO
    {.app = app_gpio_test, .name = "GPIO", .stack_size = 1024, .icon = A_GPIO_14},
#endif

#ifdef APP_ARCHIVE
    {.app = app_archive, .name = "Archive", .stack_size = 4096, .icon = A_FileManager_14},
#endif

};

const size_t FLIPPER_APPS_COUNT = sizeof(FLIPPER_APPS) / sizeof(FlipperApplication);

// On system start hooks
const FlipperOnStartHook FLIPPER_ON_SYSTEM_START[] = {
    irda_cli_init,
#ifdef APP_NFC
    nfc_cli_init,
#endif
#ifdef APP_SUBGHZ
    subghz_cli_init,
#endif
#ifdef APP_LF_RFID
    lfrfid_cli_init,
#endif
#ifdef APP_IBUTTON
    ibutton_cli_init,
#endif
#ifdef SRV_BT
    bt_cli_init,
#endif
};

const size_t FLIPPER_ON_SYSTEM_START_COUNT =
    sizeof(FLIPPER_ON_SYSTEM_START) / sizeof(FlipperOnStartHook);

// Plugin menu
const FlipperApplication FLIPPER_PLUGINS[] = {

#ifdef APP_MUSIC_PLAYER
    {.app = music_player, .name = "music player", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_FLOOPPER_BLOOPPER
    {.app = floopper_bloopper, .name = "Floopper Bloopper", .stack_size = 1024, .icon = A_Games_14},
#endif

#ifdef APP_SPEAKER_DEMO
    {.app = coreglitch_demo_0,
     .name = "coreglitch_demo_0",
     .stack_size = 1024,
     .icon = A_Plugins_14},
#endif

};

const size_t FLIPPER_PLUGINS_COUNT = sizeof(FLIPPER_PLUGINS) / sizeof(FlipperApplication);

// Plugin menu
const FlipperApplication FLIPPER_DEBUG_APPS[] = {
#ifdef APP_EXAMPLE_BLINK
    {.app = application_blink, .name = "blink", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_EXAMPLE_INPUT_DUMP
    {.app = application_input_dump, .name = "input dump", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_SD_TEST
    {.app = sd_card_test, .name = "sd_card_test", .stack_size = 4096, .icon = A_Plugins_14},
#endif

#ifdef APP_VIBRO_DEMO
    {.app = application_vibro, .name = "vibro", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_SDNFC
    {.app = sdnfc, .name = "sdnfc", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_GUI_TEST
    {.app = gui_test, .name = "gui_test", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_KEYPAD_TEST
    {.app = keypad_test, .name = "keypad_test", .icon = A_Plugins_14},
#endif

#ifdef APP_ACCESSOR
    {.app = app_accessor, .name = "accessor", .stack_size = 4096, .icon = A_Plugins_14},
#endif

#ifdef APP_UNIT_TESTS
    {.app = flipper_test_app, .name = "Unit Tests", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_IRDA_MONITOR
    {.app = irda_monitor_app, .name = "Irda Monitor", .stack_size = 1024, .icon = A_Plugins_14},
#endif

#ifdef APP_VERTICAL_SCREEN
    {.app = application_vertical_screen,
     .name = "Vertical Screen",
     .stack_size = 1024,
     .icon = A_Plugins_14},
#endif

#ifdef APP_SCENED
    {.app = scened_app, .name = "Templated Scene", .stack_size = 1024, .icon = A_Plugins_14},
#endif
};

const size_t FLIPPER_DEBUG_APPS_COUNT = sizeof(FLIPPER_DEBUG_APPS) / sizeof(FlipperApplication);

#ifdef APP_ARCHIVE
const FlipperApplication FLIPPER_ARCHIVE =
    {.app = app_archive, .name = "Archive", .stack_size = 4096, .icon = A_FileManager_14};
#endif

#ifdef SRV_DOLPHIN
const FlipperApplication FLIPPER_SCENE =
    {.app = scene_app, .name = "Scenes", .stack_size = 1024, .icon = A_Games_14};

const FlipperApplication FLIPPER_SCENE_APPS[] = {
    {.app = passport, .name = "Passport", .stack_size = 1024, .icon = A_Games_14},
    {.app = music_player, .name = "Music player", .stack_size = 1024, .icon = A_Plugins_14},
    {.app = floopper_bloopper, .name = "Floopper Bloopper", .stack_size = 1024, .icon = A_Games_14},
};

const size_t FLIPPER_SCENE_APPS_COUNT = sizeof(FLIPPER_SCENE_APPS) / sizeof(FlipperApplication);

#endif
