#include "cli_commands.h"
#include <furi-hal.h>
#include <furi-hal-gpio.h>
#include <rtc.h>
#include <task-control-block.h>
#include <time.h>
#include <notification/notification-messages.h>
#include <shci.h>

#define ENCLAVE_SIGNATURE_KEY_SLOTS 10
#define ENCLAVE_SIGNATURE_SIZE 16

static const uint8_t enclave_signature_iv[ENCLAVE_SIGNATURE_KEY_SLOTS][16] = {
    {0xac, 0x5d, 0x68, 0xb8, 0x79, 0x74, 0xfc, 0x7f, 0x45, 0x02, 0x82, 0xf1, 0x48, 0x7e, 0x75, 0x8a},
    {0x38, 0xe6, 0x6a, 0x90, 0x5e, 0x5b, 0x8a, 0xa6, 0x70, 0x30, 0x04, 0x72, 0xc2, 0x42, 0xea, 0xaf},
    {0x73, 0xd5, 0x8e, 0xfb, 0x0f, 0x4b, 0xa9, 0x79, 0x0f, 0xde, 0x0e, 0x53, 0x44, 0x7d, 0xaa, 0xfd},
    {0x3c, 0x9a, 0xf4, 0x43, 0x2b, 0xfe, 0xea, 0xae, 0x8c, 0xc6, 0xd1, 0x60, 0xd2, 0x96, 0x64, 0xa9},
    {0x10, 0xac, 0x7b, 0x63, 0x03, 0x7f, 0x43, 0x18, 0xec, 0x9d, 0x9c, 0xc4, 0x01, 0xdc, 0x35, 0xa7},
    {0x26, 0x21, 0x64, 0xe6, 0xd0, 0xf2, 0x47, 0x49, 0xdc, 0x36, 0xcd, 0x68, 0x0c, 0x91, 0x03, 0x44},
    {0x7a, 0xbd, 0xce, 0x9c, 0x24, 0x7a, 0x2a, 0xb1, 0x3c, 0x4f, 0x5a, 0x7d, 0x80, 0x3e, 0xfc, 0x0d},
    {0xcd, 0xdd, 0xd3, 0x02, 0x85, 0x65, 0x43, 0x83, 0xf9, 0xac, 0x75, 0x2f, 0x21, 0xef, 0x28, 0x6b},
    {0xab, 0x73, 0x70, 0xe8, 0xe2, 0x56, 0x0f, 0x58, 0xab, 0x29, 0xa5, 0xb1, 0x13, 0x47, 0x5e, 0xe8},
    {0x4f, 0x3c, 0x43, 0x77, 0xde, 0xed, 0x79, 0xa1, 0x8d, 0x4c, 0x1f, 0xfd, 0xdb, 0x96, 0x87, 0x2e},
};

static const uint8_t enclave_signature_input[ENCLAVE_SIGNATURE_KEY_SLOTS][ENCLAVE_SIGNATURE_SIZE] = {
    {0x9f, 0x5c, 0xb1, 0x43, 0x17, 0x53, 0x18, 0x8c, 0x66, 0x3d, 0x39, 0x45, 0x90, 0x13, 0xa9, 0xde},
    {0xc5, 0x98, 0xe9, 0x17, 0xb8, 0x97, 0x9e, 0x03, 0x33, 0x14, 0x13, 0x8f, 0xce, 0x74, 0x0d, 0x54},
    {0x34, 0xba, 0x99, 0x59, 0x9f, 0x70, 0x67, 0xe9, 0x09, 0xee, 0x64, 0x0e, 0xb3, 0xba, 0xfb, 0x75},
    {0xdc, 0xfa, 0x6c, 0x9a, 0x6f, 0x0a, 0x3e, 0xdc, 0x42, 0xf6, 0xae, 0x0d, 0x3c, 0xf7, 0x83, 0xaf},
    {0xea, 0x2d, 0xe3, 0x1f, 0x02, 0x99, 0x1a, 0x7e, 0x6d, 0x93, 0x4c, 0xb5, 0x42, 0xf0, 0x7a, 0x9b},
    {0x53, 0x5e, 0x04, 0xa2, 0x49, 0xa0, 0x73, 0x49, 0x56, 0xb0, 0x88, 0x8c, 0x12, 0xa0, 0xe4, 0x18},
    {0x7d, 0xa7, 0xc5, 0x21, 0x7f, 0x12, 0x95, 0xdd, 0x4d, 0x77, 0x01, 0xfa, 0x71, 0x88, 0x2b, 0x7f},
    {0xdc, 0x9b, 0xc5, 0xa7, 0x6b, 0x84, 0x5c, 0x37, 0x7c, 0xec, 0x05, 0xa1, 0x9f, 0x91, 0x17, 0x3b},
    {0xea, 0xcf, 0xd9, 0x9b, 0x86, 0xcd, 0x2b, 0x43, 0x54, 0x45, 0x82, 0xc6, 0xfe, 0x73, 0x1a, 0x1a},
    {0x77, 0xb8, 0x1b, 0x90, 0xb4, 0xb7, 0x32, 0x76, 0x8f, 0x8a, 0x57, 0x06, 0xc7, 0xdd, 0x08, 0x90},
};

static const uint8_t enclave_signature_expected[ENCLAVE_SIGNATURE_KEY_SLOTS][ENCLAVE_SIGNATURE_SIZE] = {
    {0xe9, 0x9a, 0xce, 0xe9, 0x4d, 0xe1, 0x7f, 0x55, 0xcb, 0x8a, 0xbf, 0xf2, 0x4d, 0x98, 0x27, 0x67},
    {0x34, 0x27, 0xa7, 0xea, 0xa8, 0x98, 0x66, 0x9b, 0xed, 0x43, 0xd3, 0x93, 0xb5, 0xa2, 0x87, 0x8e},
    {0x6c, 0xf3, 0x01, 0x78, 0x53, 0x1b, 0x11, 0x32, 0xf0, 0x27, 0x2f, 0xe3, 0x7d, 0xa6, 0xe2, 0xfd},
    {0xdf, 0x7f, 0x37, 0x65, 0x2f, 0xdb, 0x7c, 0xcf, 0x5b, 0xb6, 0xe4, 0x9c, 0x63, 0xc5, 0x0f, 0xe0},
    {0x9b, 0x5c, 0xee, 0x44, 0x0e, 0xd1, 0xcb, 0x5f, 0x28, 0x9f, 0x12, 0x17, 0x59, 0x64, 0x40, 0xbb},
    {0x94, 0xc2, 0x09, 0x98, 0x62, 0xa7, 0x2b, 0x93, 0xed, 0x36, 0x1f, 0x10, 0xbc, 0x26, 0xbd, 0x41},
    {0x4d, 0xb2, 0x2b, 0xc5, 0x96, 0x47, 0x61, 0xf4, 0x16, 0xe0, 0x81, 0xc3, 0x8e, 0xb9, 0x9c, 0x9b},
    {0xc3, 0x6b, 0x83, 0x55, 0x90, 0x38, 0x0f, 0xea, 0xd1, 0x65, 0xbf, 0x32, 0x4f, 0x8e, 0x62, 0x5b},
    {0x8d, 0x5e, 0x27, 0xbc, 0x14, 0x4f, 0x08, 0xa8, 0x2b, 0x14, 0x89, 0x5e, 0xdf, 0x77, 0x04, 0x31},
    {0xc9, 0xf7, 0x03, 0xf1, 0x6c, 0x65, 0xad, 0x49, 0x74, 0xbe, 0x00, 0x54, 0xfd, 0xa6, 0x9c, 0x32},
};

/* 
 * Device Info Command
 * This command is intended to be used by humans and machines
 * Keys and values format MUST NOT BE changed
 */
void cli_command_device_info(Cli* cli, string_t args, void* context) {
    // Device Info version
    printf("device_info_major       : %d\r\n", 2);
    printf("device_info_minor       : %d\r\n", 0);
    // Model name
    printf("hardware_model          : %s\r\n", furi_hal_version_get_model_name());

    // Unique ID
    printf("hardware_uid            : ");
    const uint8_t* uid = furi_hal_version_uid();
    for(size_t i = 0; i < furi_hal_version_uid_size(); i++) {
        printf("%02X", uid[i]);
    }
    printf("\r\n");

    // OTP Revision
    printf("hardware_otp_ver        : %d\r\n", furi_hal_version_get_otp_version());
    printf("hardware_timestamp      : %lu\r\n", furi_hal_version_get_hw_timestamp());

    // Board Revision
    printf("hardware_ver            : %d\r\n", furi_hal_version_get_hw_version());
    printf("hardware_target         : %d\r\n", furi_hal_version_get_hw_target());
    printf("hardware_body           : %d\r\n", furi_hal_version_get_hw_body());
    printf("hardware_connect        : %d\r\n", furi_hal_version_get_hw_connect());
    printf("hardware_display        : %d\r\n", furi_hal_version_get_hw_display());

    // Board Personification
    printf("hardware_color          : %d\r\n", furi_hal_version_get_hw_color());
    printf("hardware_region         : %d\r\n", furi_hal_version_get_hw_region());
    const char* name = furi_hal_version_get_name_ptr();
    if(name) {
        printf("hardware_name           : %s\r\n", name);
    }

    // Bootloader Version
    const Version* bootloader_version = furi_hal_version_get_bootloader_version();
    if(bootloader_version) {
        printf("bootloader_commit       : %s\r\n", version_get_githash(bootloader_version));
        printf("bootloader_branch       : %s\r\n", version_get_gitbranch(bootloader_version));
        printf("bootloader_branch_num   : %s\r\n", version_get_gitbranchnum(bootloader_version));
        printf("bootloader_version      : %s\r\n", version_get_version(bootloader_version));
        printf("bootloader_build_date   : %s\r\n", version_get_builddate(bootloader_version));
        printf("bootloader_target       : %d\r\n", version_get_target(bootloader_version));
    }

    // Firmware version
    const Version* firmware_version = furi_hal_version_get_firmware_version();
    if(firmware_version) {
        printf("firmware_commit         : %s\r\n", version_get_githash(firmware_version));
        printf("firmware_branch         : %s\r\n", version_get_gitbranch(firmware_version));
        printf("firmware_branch_num     : %s\r\n", version_get_gitbranchnum(firmware_version));
        printf("firmware_version        : %s\r\n", version_get_version(firmware_version));
        printf("firmware_build_date     : %s\r\n", version_get_builddate(firmware_version));
        printf("firmware_target         : %d\r\n", version_get_target(firmware_version));
    }

    WirelessFwInfo_t pWirelessInfo;
    if(furi_hal_bt_is_alive() && SHCI_GetWirelessFwInfo(&pWirelessInfo) == SHCI_Success) {
        printf("radio_alive             : true\r\n");
        // FUS Info
        printf("radio_fus_major         : %d\r\n", pWirelessInfo.FusVersionMajor);
        printf("radio_fus_minor         : %d\r\n", pWirelessInfo.FusVersionMinor);
        printf("radio_fus_sub           : %d\r\n", pWirelessInfo.FusVersionSub);
        printf("radio_fus_sram2b        : %dK\r\n", pWirelessInfo.FusMemorySizeSram2B);
        printf("radio_fus_sram2a        : %dK\r\n", pWirelessInfo.FusMemorySizeSram2A);
        printf("radio_fus_flash         : %dK\r\n", pWirelessInfo.FusMemorySizeFlash * 4);
        // Stack Info
        printf("radio_stack_type        : %d\r\n", pWirelessInfo.StackType);
        printf("radio_stack_major       : %d\r\n", pWirelessInfo.VersionMajor);
        printf("radio_stack_minor       : %d\r\n", pWirelessInfo.VersionMinor);
        printf("radio_stack_sub         : %d\r\n", pWirelessInfo.VersionSub);
        printf("radio_stack_branch      : %d\r\n", pWirelessInfo.VersionBranch);
        printf("radio_stack_release     : %d\r\n", pWirelessInfo.VersionReleaseType);
        printf("radio_stack_sram2b      : %dK\r\n", pWirelessInfo.MemorySizeSram2B);
        printf("radio_stack_sram2a      : %dK\r\n", pWirelessInfo.MemorySizeSram2A);
        printf("radio_stack_sram1       : %dK\r\n", pWirelessInfo.MemorySizeSram1);
        printf("radio_stack_flash       : %dK\r\n", pWirelessInfo.MemorySizeFlash * 4);
        // Mac address
        printf("radio_ble_mac           : ");
        const uint8_t* ble_mac = furi_hal_version_get_ble_mac();
        for(size_t i = 0; i < 6; i++) {
            printf("%02X", ble_mac[i]);
        }
        printf("\r\n");

        // Signature verification
        uint8_t buffer[ENCLAVE_SIGNATURE_SIZE];
        size_t enclave_valid_keys = 0;
        for(size_t key_slot = 0; key_slot < ENCLAVE_SIGNATURE_KEY_SLOTS; key_slot++) {
            if(furi_hal_crypto_store_load_key(key_slot + 1, enclave_signature_iv[key_slot])) {
                if(furi_hal_crypto_encrypt(
                       enclave_signature_input[key_slot], buffer, ENCLAVE_SIGNATURE_SIZE)) {
                    enclave_valid_keys += memcmp(
                                              buffer,
                                              enclave_signature_expected[key_slot],
                                              ENCLAVE_SIGNATURE_SIZE) == 0;
                }
                furi_hal_crypto_store_unload_key(key_slot + 1);
            }
        }
        printf("enclave_valid_keys      : %d\r\n", enclave_valid_keys);
        printf(
            "enclave_valid           : %s\r\n",
            (enclave_valid_keys == ENCLAVE_SIGNATURE_KEY_SLOTS) ? "true" : "false");
    } else {
        printf("radio_alive             : false\r\n");
    }
}

void cli_command_help(Cli* cli, string_t args, void* context) {
    (void)args;
    printf("Commands we have:");

    // Command count
    const size_t commands_count = CliCommandTree_size(cli->commands);
    const size_t commands_count_mid = commands_count / 2 + commands_count % 2;

    // Use 2 iterators from start and middle to show 2 columns
    CliCommandTree_it_t it_left;
    CliCommandTree_it(it_left, cli->commands);
    CliCommandTree_it_t it_right;
    CliCommandTree_it(it_right, cli->commands);
    for(size_t i = 0; i < commands_count_mid; i++) CliCommandTree_next(it_right);

    // Iterate throw tree
    for(size_t i = 0; i < commands_count_mid; i++) {
        printf("\r\n");
        // Left Column
        if(!CliCommandTree_end_p(it_left)) {
            printf("%-30s", string_get_cstr(*CliCommandTree_ref(it_left)->key_ptr));
            CliCommandTree_next(it_left);
        }
        // Right Column
        if(!CliCommandTree_end_p(it_right)) {
            printf("%s", string_get_cstr(*CliCommandTree_ref(it_right)->key_ptr));
            CliCommandTree_next(it_right);
        }
    };

    if(string_size(args) > 0) {
        cli_nl();
        printf("Also I have no clue what '");
        printf("%s", string_get_cstr(args));
        printf("' is.");
    }
}

void cli_command_date(Cli* cli, string_t args, void* context) {
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    if(string_size(args) > 0) {
        uint16_t Hours, Minutes, Seconds, Month, Date, Year, WeekDay;
        int ret = sscanf(
            string_get_cstr(args),
            "%hu:%hu:%hu %hu-%hu-%hu %hu",
            &Hours,
            &Minutes,
            &Seconds,
            &Month,
            &Date,
            &Year,
            &WeekDay);
        if(ret == 7) {
            time.Hours = Hours;
            time.Minutes = Minutes;
            time.Seconds = Seconds;
            time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
            time.StoreOperation = RTC_STOREOPERATION_RESET;
            date.WeekDay = WeekDay;
            date.Month = Month;
            date.Date = Date;
            date.Year = Year - 2000;
            HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
            HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);

            // Verification
            HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
            HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
            printf(
                "New time is: %.2d:%.2d:%.2d %.2d-%.2d-%.2d %d",
                time.Hours,
                time.Minutes,
                time.Seconds,
                date.Month,
                date.Date,
                2000 + date.Year,
                date.WeekDay);
        } else {
            printf(
                "Invalid time format, use `hh:mm:ss MM-DD-YYYY WD`. sscanf %d %s",
                ret,
                string_get_cstr(args));
            return;
        }
    } else {
        // TODO add get_datetime to core, not use HAL here
        // READ ORDER MATTERS! Time then date.
        HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
        printf(
            "%.2d:%.2d:%.2d %.2d-%.2d-%.2d %d",
            time.Hours,
            time.Minutes,
            time.Seconds,
            date.Month,
            date.Date,
            2000 + date.Year,
            date.WeekDay);
    }
}

void cli_command_log(Cli* cli, string_t args, void* context) {
    furi_stdglue_set_global_stdout_callback(cli_stdout_callback);
    printf("Press any key to stop...\r\n");
    cli_getc(cli);
    furi_stdglue_set_global_stdout_callback(NULL);
}

void cli_command_vibro(Cli* cli, string_t args, void* context) {
    if(!string_cmp(args, "0")) {
        NotificationApp* notification = furi_record_open("notification");
        notification_message_block(notification, &sequence_reset_vibro);
        furi_record_close("notification");
    } else if(!string_cmp(args, "1")) {
        NotificationApp* notification = furi_record_open("notification");
        notification_message_block(notification, &sequence_set_vibro_on);
        furi_record_close("notification");
    } else {
        cli_print_usage("vibro", "<1|0>", string_get_cstr(args));
    }
}

void cli_command_led(Cli* cli, string_t args, void* context) {
    // Get first word as light name
    NotificationMessage notification_led_message;
    string_t light_name;
    string_init(light_name);
    size_t ws = string_search_char(args, ' ');
    if(ws == STRING_FAILURE) {
        cli_print_usage("led", "<r|g|b|bl> <0-255>", string_get_cstr(args));
        string_clear(light_name);
        return;
    } else {
        string_set_n(light_name, args, 0, ws);
        string_right(args, ws);
        string_strim(args);
    }
    // Check light name
    if(!string_cmp(light_name, "r")) {
        notification_led_message.type = NotificationMessageTypeLedRed;
    } else if(!string_cmp(light_name, "g")) {
        notification_led_message.type = NotificationMessageTypeLedGreen;
    } else if(!string_cmp(light_name, "b")) {
        notification_led_message.type = NotificationMessageTypeLedBlue;
    } else if(!string_cmp(light_name, "bl")) {
        notification_led_message.type = NotificationMessageTypeLedDisplay;
    } else {
        cli_print_usage("led", "<r|g|b|bl> <0-255>", string_get_cstr(args));
        string_clear(light_name);
        return;
    }
    string_clear(light_name);
    // Read light value from the rest of the string
    char* end_ptr;
    uint32_t value = strtoul(string_get_cstr(args), &end_ptr, 0);
    if(!(value < 256 && *end_ptr == '\0')) {
        cli_print_usage("led", "<r|g|b|bl> <0-255>", string_get_cstr(args));
        return;
    }

    // Set led value
    notification_led_message.data.led.value = value;

    // Form notification sequence
    const NotificationSequence notification_sequence = {
        &notification_led_message,
        NULL,
    };

    // Send notification
    NotificationApp* notification = furi_record_open("notification");
    notification_internal_message_block(notification, &notification_sequence);
    furi_record_close("notification");
}

void cli_command_gpio_set(Cli* cli, string_t args, void* context) {
    char pin_names[][4] = {
        "PC0",
        "PC1",
        "PC3",
        "PB2",
        "PB3",
        "PA4",
        "PA6",
        "PA7",
#ifdef FURI_DEBUG
        "PA0",
        "PB7",
        "PB8",
        "PB9"
#endif
    };
    GpioPin gpio[] = {
        {.port = GPIOC, .pin = LL_GPIO_PIN_0},
        {.port = GPIOC, .pin = LL_GPIO_PIN_1},
        {.port = GPIOC, .pin = LL_GPIO_PIN_3},
        {.port = GPIOB, .pin = LL_GPIO_PIN_2},
        {.port = GPIOB, .pin = LL_GPIO_PIN_3},
        {.port = GPIOA, .pin = LL_GPIO_PIN_4},
        {.port = GPIOA, .pin = LL_GPIO_PIN_6},
        {.port = GPIOA, .pin = LL_GPIO_PIN_7},
#ifdef FURI_DEBUG
        {.port = GPIOA, .pin = LL_GPIO_PIN_0}, // IR_RX (PA0)
        {.port = GPIOB, .pin = LL_GPIO_PIN_7}, // UART RX (PB7)
        {.port = GPIOB, .pin = LL_GPIO_PIN_8}, // SPEAKER (PB8)
        {.port = GPIOB, .pin = LL_GPIO_PIN_9}, // IR_TX (PB9)
#endif
    };
    uint8_t num = 0;
    bool pin_found = false;

    // Get first word as pin name
    string_t pin_name;
    string_init(pin_name);
    size_t ws = string_search_char(args, ' ');
    if(ws == STRING_FAILURE) {
        cli_print_usage("gpio_set", "<pin_name> <0|1>", string_get_cstr(args));
        string_clear(pin_name);
        return;
    } else {
        string_set_n(pin_name, args, 0, ws);
        string_right(args, ws);
        string_strim(args);
    }
    // Search correct pin name
    for(num = 0; num < sizeof(pin_names) / sizeof(char*); num++) {
        if(!string_cmp(pin_name, pin_names[num])) {
            pin_found = true;
            break;
        }
    }
    if(!pin_found) {
        printf("Wrong pin name. Available pins: ");
        for(uint8_t i = 0; i < sizeof(pin_names) / sizeof(char*); i++) {
            printf("%s ", pin_names[i]);
        }
        string_clear(pin_name);
        return;
    }
    string_clear(pin_name);
    // Read "0" or "1" as second argument to set or reset pin
    if(!string_cmp(args, "0")) {
        LL_GPIO_SetPinMode(gpio[num].port, gpio[num].pin, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetPinOutputType(gpio[num].port, gpio[num].pin, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_ResetOutputPin(gpio[num].port, gpio[num].pin);
    } else if(!string_cmp(args, "1")) {
#ifdef FURI_DEBUG
        if(num == 8) { // PA0
            printf(
                "Setting PA0 pin HIGH with TSOP connected can damage IR receiver. Are you sure you want to continue? (y/n)?\r\n");
            char c = cli_getc(cli);
            if(c != 'y' && c != 'Y') {
                printf("Cancelled.\r\n");
                return;
            }
        }
#endif

        LL_GPIO_SetPinMode(gpio[num].port, gpio[num].pin, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetPinOutputType(gpio[num].port, gpio[num].pin, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetOutputPin(gpio[num].port, gpio[num].pin);
    } else {
        printf("Wrong 2nd argument. Use \"1\" to set, \"0\" to reset");
    }
    return;
}

void cli_command_ps(Cli* cli, string_t args, void* context) {
    const uint8_t threads_num_max = 32;
    osThreadId_t threads_id[threads_num_max];
    uint8_t thread_num = osThreadEnumerate(threads_id, threads_num_max);
    printf(
        "%-20s %-14s %-8s %-8s %s\r\n", "Name", "Stack start", "Heap", "Stack", "Stack min free");
    for(uint8_t i = 0; i < thread_num; i++) {
        TaskControlBlock* tcb = (TaskControlBlock*)threads_id[i];
        printf(
            "%-20s 0x%-12lx %-8d %-8ld %-8ld\r\n",
            osThreadGetName(threads_id[i]),
            (uint32_t)tcb->pxStack,
            memmgr_heap_get_thread_memory(threads_id[i]),
            (uint32_t)(tcb->pxEndOfStack - tcb->pxStack + 1) * sizeof(StackType_t),
            osThreadGetStackSpace(threads_id[i]));
    }
    printf("\r\nTotal: %d", thread_num);
}

void cli_command_free(Cli* cli, string_t args, void* context) {
    printf("Free heap size: %d\r\n", memmgr_get_free_heap());
    printf("Minimum heap size: %d\r\n", memmgr_get_minimum_free_heap());
    printf("Maximum heap block: %d\r\n", memmgr_heap_get_max_free_block());
}

void cli_command_free_blocks(Cli* cli, string_t args, void* context) {
    memmgr_heap_printf_free_blocks();
}

void cli_command_i2c(Cli* cli, string_t args, void* context) {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    uint8_t test = 0;
    printf("Scanning external i2c on PC0(SCL)/PC1(SDA)\r\n"
           "Clock: 100khz, 7bit address\r\n"
           "\r\n");
    printf("  | 0 1 2 3 4 5 6 7 8 9 A B C D E F\r\n");
    printf("--+--------------------------------\r\n");
    for(uint8_t row = 0; row < 0x8; row++) {
        printf("%x | ", row);
        for(uint8_t column = 0; column <= 0xF; column++) {
            bool ret =
                furi_hal_i2c_rx(&furi_hal_i2c_handle_external, (row << 4) + column, &test, 1, 2);
            printf("%c ", ret ? '#' : '-');
        }
        printf("\r\n");
    }
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);
}

void cli_commands_init(Cli* cli) {
    cli_add_command(cli, "!", CliCommandFlagParallelSafe, cli_command_device_info, NULL);
    cli_add_command(cli, "device_info", CliCommandFlagParallelSafe, cli_command_device_info, NULL);

    cli_add_command(cli, "?", CliCommandFlagParallelSafe, cli_command_help, NULL);
    cli_add_command(cli, "help", CliCommandFlagParallelSafe, cli_command_help, NULL);

    cli_add_command(cli, "date", CliCommandFlagParallelSafe, cli_command_date, NULL);
    cli_add_command(cli, "log", CliCommandFlagParallelSafe, cli_command_log, NULL);
    cli_add_command(cli, "ps", CliCommandFlagParallelSafe, cli_command_ps, NULL);
    cli_add_command(cli, "free", CliCommandFlagParallelSafe, cli_command_free, NULL);
    cli_add_command(cli, "free_blocks", CliCommandFlagParallelSafe, cli_command_free_blocks, NULL);

    cli_add_command(cli, "vibro", CliCommandFlagDefault, cli_command_vibro, NULL);
    cli_add_command(cli, "led", CliCommandFlagDefault, cli_command_led, NULL);
    cli_add_command(cli, "gpio_set", CliCommandFlagDefault, cli_command_gpio_set, NULL);
    cli_add_command(cli, "i2c", CliCommandFlagDefault, cli_command_i2c, NULL);
}
