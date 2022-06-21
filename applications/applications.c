#include "applications.h"
#include <assets_icons.h>

// Services
extern int32_t rpc_srv(void* p);
extern int32_t bt_srv(void* p);
extern int32_t cli_srv(void* p);
extern int32_t dialogs_srv(void* p);
extern int32_t dolphin_srv(void* p);
extern int32_t gui_srv(void* p);
extern int32_t input_srv(void* p);
extern int32_t loader_srv(void* p);
extern int32_t notification_srv(void* p);
extern int32_t power_srv(void* p);
extern int32_t storage_srv(void* p);
extern int32_t desktop_srv(void* p);
extern int32_t updater_srv(void* p);

// Apps
extern int32_t accessor_app(void* p);
extern int32_t archive_app(void* p);
extern int32_t bad_usb_app(void* p);
extern int32_t u2f_app(void* p);
extern int32_t uart_echo_app(void* p);
extern int32_t blink_test_app(void* p);
extern int32_t bt_debug_app(void* p);
extern int32_t delay_test_app(void* p);
extern int32_t display_test_app(void* p);
extern int32_t gpio_app(void* p);
extern int32_t ibutton_app(void* p);
extern int32_t infrared_app(void* p);
extern int32_t keypad_test_app(void* p);
extern int32_t lfrfid_app(void* p);
extern int32_t lfrfid_debug_app(void* p);
extern int32_t nfc_app(void* p);
extern int32_t passport_app(void* p);
extern int32_t scened_app(void* p);
extern int32_t storage_test_app(void* p);
extern int32_t subghz_app(void* p);
extern int32_t usb_mouse_app(void* p);
extern int32_t usb_test_app(void* p);
extern int32_t vibro_test_app(void* p);
extern int32_t bt_hid_app(void* p);
extern int32_t battery_test_app(void* p);
extern int32_t text_box_test_app(void* p);
extern int32_t file_browser_app(void* p);

// Plugins
extern int32_t music_player_app(void* p);
extern int32_t snake_game_app(void* p);

// On system start hooks declaration
extern void bt_on_system_start();
extern void crypto_on_system_start();
extern void ibutton_on_system_start();
extern void infrared_on_system_start();
extern void lfrfid_on_system_start();
extern void music_player_on_system_start();
extern void nfc_on_system_start();
extern void storage_on_system_start();
extern void subghz_on_system_start();
extern void power_on_system_start();
extern void unit_tests_on_system_start();
extern void updater_on_system_start();

// Settings
extern int32_t notification_settings_app(void* p);
extern int32_t storage_settings_app(void* p);
extern int32_t bt_settings_app(void* p);
extern int32_t desktop_settings_app(void* p);
extern int32_t about_settings_app(void* p);
extern int32_t power_settings_app(void* p);
extern int32_t system_settings_app(void* p);

const FlipperApplication FLIPPER_SERVICES[] = {
/* Services */
#ifdef SRV_RPC
    {.app = rpc_srv,
     .name = "RpcSrv",
     .stack_size = 1024 * 4,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_BT
    {.app = bt_srv,
     .name = "BtSrv",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_CLI
    {.app = cli_srv,
     .name = "CliSrv",
     .stack_size = 4096,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_DIALOGS
    {.app = dialogs_srv,
     .name = "DialogsSrv",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_DOLPHIN
    {.app = dolphin_srv,
     .name = "DolphinSrv",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_DESKTOP
#ifdef SRV_UPDATER
#error SRV_UPDATER and SRV_DESKTOP are mutually exclusive!
#endif
    {.app = desktop_srv,
     .name = "DesktopSrv",
     .stack_size = 2048,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_GUI
    {.app = gui_srv,
     .name = "GuiSrv",
     .stack_size = 2048,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_INPUT
    {.app = input_srv,
     .name = "InputSrv",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_LOADER
    {.app = loader_srv,
     .name = "LoaderSrv",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_NOTIFICATION
    {.app = notification_srv,
     .name = "NotificationSrv",
     .stack_size = 1536,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_POWER
    {.app = power_srv,
     .name = "PowerSrv",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_STORAGE
    {.app = storage_srv,
     .name = "StorageSrv",
     .stack_size = 3072,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_UPDATER
#ifdef SRV_DESKTOP
#error SRV_UPDATER and SRV_DESKTOP are mutually exclusive!
#endif
    {.app = updater_srv,
     .name = "UpdaterSrv",
     .stack_size = 2048,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif
};

const size_t FLIPPER_SERVICES_COUNT = COUNT_OF(FLIPPER_SERVICES);

const FlipperApplication FLIPPER_SYSTEM_APPS[] = {
#ifdef APP_UPDATER
#ifdef SRV_UPDATER
#error APP_UPDATER and SRV_UPDATER are mutually exclusive!
#endif
    {.app = updater_srv,
     .name = "UpdaterApp",
     .stack_size = 2048,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif
};

const size_t FLIPPER_SYSTEM_APPS_COUNT = COUNT_OF(FLIPPER_SYSTEM_APPS);

// Main menu APP
const FlipperApplication FLIPPER_APPS[] = {

#ifdef APP_SUBGHZ
    {.app = subghz_app,
     .name = "Sub-GHz",
     .stack_size = 2048,
     .icon = &A_Sub1ghz_14,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_LF_RFID
    {.app = lfrfid_app,
     .name = "125 kHz RFID",
     .stack_size = 2048,
     .icon = &A_125khz_14,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_NFC
    {.app = nfc_app,
     .name = "NFC",
     .stack_size = 4096,
     .icon = &A_NFC_14,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_INFRARED
    {.app = infrared_app,
     .name = "Infrared",
     .stack_size = 1024 * 3,
     .icon = &A_Infrared_14,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_GPIO
    {.app = gpio_app,
     .name = "GPIO",
     .stack_size = 1024,
     .icon = &A_GPIO_14,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_IBUTTON
    {.app = ibutton_app,
     .name = "iButton",
     .stack_size = 2048,
     .icon = &A_iButton_14,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_BAD_USB
    {.app = bad_usb_app,
     .name = "Bad USB",
     .stack_size = 2048,
     .icon = &A_BadUsb_14,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_U2F
    {.app = u2f_app,
     .name = "U2F",
     .stack_size = 2048,
     .icon = &A_U2F_14,
     .flags = FlipperApplicationFlagDefault},
#endif

};

const size_t FLIPPER_APPS_COUNT = COUNT_OF(FLIPPER_APPS);

// On system start hooks
const FlipperOnStartHook FLIPPER_ON_SYSTEM_START[] = {
    crypto_on_system_start,

#ifdef APP_INFRARED
    infrared_on_system_start,
#endif

#ifdef APP_MUSIC_PLAYER
    music_player_on_system_start,
#endif

#ifdef APP_NFC
    nfc_on_system_start,
#endif

#ifdef APP_SUBGHZ
    subghz_on_system_start,
#endif

#ifdef APP_LF_RFID
    lfrfid_on_system_start,
#endif

#ifdef APP_IBUTTON
    ibutton_on_system_start,
#endif

#ifdef SRV_BT
    bt_on_system_start,
#endif

#ifdef SRV_POWER
    power_on_system_start,
#endif

#ifdef SRV_STORAGE
    storage_on_system_start,
#endif

#ifdef APP_UNIT_TESTS
    unit_tests_on_system_start,
#endif

#ifdef APP_UPDATER
    updater_on_system_start,
#endif
};

const size_t FLIPPER_ON_SYSTEM_START_COUNT = COUNT_OF(FLIPPER_ON_SYSTEM_START);

// Plugin menu
const FlipperApplication FLIPPER_PLUGINS[] = {
#ifdef APP_BLE_HID
    {.app = bt_hid_app,
     .name = "Bluetooth Remote",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_MUSIC_PLAYER
    {.app = music_player_app,
     .name = "Music Player",
     .stack_size = 2048,
     .icon = &A_Plugins_14,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_SNAKE_GAME
    {.app = snake_game_app,
     .name = "Snake Game",
     .stack_size = 1024,
     .icon = &A_Plugins_14,
     .flags = FlipperApplicationFlagDefault},
#endif
};

const size_t FLIPPER_PLUGINS_COUNT = COUNT_OF(FLIPPER_PLUGINS);

// Plugin menu
const FlipperApplication FLIPPER_DEBUG_APPS[] = {
#ifdef APP_BLINK
    {.app = blink_test_app,
     .name = "Blink Test",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_VIBRO_TEST
    {.app = vibro_test_app,
     .name = "Vibro Test",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_KEYPAD_TEST
    {.app = keypad_test_app,
     .name = "Keypad Test",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_ACCESSOR
    {.app = accessor_app,
     .name = "Accessor",
     .stack_size = 4096,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_USB_TEST
    {.app = usb_test_app,
     .name = "USB Test",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_USB_MOUSE
    {.app = usb_mouse_app,
     .name = "USB Mouse Demo",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_UART_ECHO
    {.app = uart_echo_app,
     .name = "Uart Echo",
     .stack_size = 2048,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_LF_RFID
    {.app = lfrfid_debug_app,
     .name = "LF-RFID Debug",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_BT
    {.app = bt_debug_app,
     .name = "Bluetooth Debug",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_UNIT_TESTS
    {.app = delay_test_app,
     .name = "Delay Test",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_DISPLAY_TEST
    {.app = display_test_app,
     .name = "Display Test",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_FILE_BROWSER_TEST
    {.app = file_browser_app,
     .name = "File Browser test",
     .stack_size = 2048,
     .icon = &A_BadUsb_14,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_BATTERY_TEST
    {.app = battery_test_app,
     .name = "Battery Test",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_TEXT_BOX_TEST
    {.app = text_box_test_app,
     .name = "Text Box Test",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif
};

const size_t FLIPPER_DEBUG_APPS_COUNT = COUNT_OF(FLIPPER_DEBUG_APPS);

#ifdef APP_ARCHIVE
const FlipperApplication FLIPPER_ARCHIVE = {
    .app = archive_app,
    .name = "Archive",
    .stack_size = 4096,
    .icon = &A_FileManager_14,
    .flags = FlipperApplicationFlagDefault};
#endif

// Settings menu
const FlipperApplication FLIPPER_SETTINGS_APPS[] = {
#ifdef SRV_BT
    {.app = bt_settings_app,
     .name = "Bluetooth",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_NOTIFICATION
    {.app = notification_settings_app,
     .name = "LCD and Notifications",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_STORAGE
    {.app = storage_settings_app,
     .name = "Storage",
     .stack_size = 2048,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_POWER
    {.app = power_settings_app,
     .name = "Power",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagInsomniaSafe},
#endif

#ifdef SRV_DESKTOP
    {.app = desktop_settings_app,
     .name = "Desktop",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_PASSPORT
    {.app = passport_app,
     .name = "Passport",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef SRV_GUI
    {.app = system_settings_app,
     .name = "System",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif

#ifdef APP_ABOUT
    {.app = about_settings_app,
     .name = "About",
     .stack_size = 1024,
     .icon = NULL,
     .flags = FlipperApplicationFlagDefault},
#endif
};

const size_t FLIPPER_SETTINGS_APPS_COUNT = COUNT_OF(FLIPPER_SETTINGS_APPS);
