#include "cli_commands.h"
#include <api-hal.h>
#include <api-hal-gpio.h>
#include <rtc.h>
#include <task-control-block.h>

void cli_command_help(string_t args, void* context) {
    (void)args;
    Cli* cli = context;
    printf("Commands we have:");

    furi_check(osMutexAcquire(cli->mutex, osWaitForever) == osOK);
    CliCommandDict_it_t it;
    for(CliCommandDict_it(it, cli->commands); !CliCommandDict_end_p(it); CliCommandDict_next(it)) {
        CliCommandDict_itref_t* ref = CliCommandDict_ref(it);
        printf(" ");
        printf(string_get_cstr(ref->key));
    };
    furi_check(osMutexRelease(cli->mutex) == osOK);

    if(string_size(args) > 0) {
        cli_nl();
        printf("Also I have no clue what '");
        printf(string_get_cstr(args));
        printf("' is.");
    }
}

void cli_command_version(string_t args, void* context) {
    (void)args;
    (void)context;
    printf("Bootloader\r\n");
    cli_print_version(api_hal_version_get_boot_version());

    printf("Firmware\r\n");
    cli_print_version(api_hal_version_get_fw_version());
}

void cli_command_uuid(string_t args, void* context) {
    (void)args;
    (void)context;
    size_t uid_size = api_hal_uid_size();
    const uint8_t* uid = api_hal_uid();

    string_t byte_str;
    string_init(byte_str);
    string_cat_printf(byte_str, "UID:");
    for(size_t i = 0; i < uid_size; i++) {
        uint8_t uid_byte = uid[i];
        string_cat_printf(byte_str, "%02X", uid_byte);
    }
    printf(string_get_cstr(byte_str));
}

void cli_command_date(string_t args, void* context) {
    RTC_DateTypeDef date;
    RTC_TimeTypeDef time;

    // TODO add get_datetime to core, not use HAL here
    // READ ORDER MATTERS! Time then date.
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

    string_t datetime_str;
    string_init(datetime_str);

    string_cat_printf(datetime_str, "%.2d:%.2d:%.2d ", time.Hours, time.Minutes, time.Seconds);
    string_cat_printf(datetime_str, "%.2d-%.2d-%.2d", date.Month, date.Date, 2000 + date.Year);

    printf(string_get_cstr(datetime_str));

    string_clear(datetime_str);
}

void cli_command_log(string_t args, void* context) {
    Cli* cli = context;
    furi_stdglue_set_global_stdout_callback(cli_stdout_callback);
    printf("Press any key to stop...\r\n");
    cli_getc(cli);
    furi_stdglue_set_global_stdout_callback(NULL);
}

void cli_command_vibro(string_t args, void* context) {
    if(!string_cmp(args, "0")) {
        api_hal_vibro_on(false);
    } else if(!string_cmp(args, "1")) {
        api_hal_vibro_on(true);
    } else {
        printf("Wrong input");
    }
}

void cli_command_led(string_t args, void* context) {
    // Get first word as light name
    Light light;
    string_t light_name;
    string_init(light_name);
    size_t ws = string_search_char(args, ' ');
    if(ws == STRING_FAILURE) {
        printf("Wrong input");
        string_clear(light_name);
        return;
    } else {
        string_set_n(light_name, args, 0, ws);
        string_right(args, ws);
        string_strim(args);
    }
    // Check light name
    if(!string_cmp(light_name, "r")) {
        light = LightRed;
    } else if(!string_cmp(light_name, "g")) {
        light = LightGreen;
    } else if(!string_cmp(light_name, "b")) {
        light = LightBlue;
    } else if(!string_cmp(light_name, "bl")) {
        light = LightBacklight;
    } else {
        printf("Wrong argument");
        string_clear(light_name);
        return;
    }
    string_clear(light_name);
    // Read light value from the rest of the string
    char* end_ptr;
    uint32_t value = strtoul(string_get_cstr(args), &end_ptr, 0);
    if(!(value < 256 && *end_ptr == '\0')) {
        printf("Wrong argument");
        return;
    }
    api_hal_light_set(light, value);
}

void cli_command_gpio_set(string_t args, void* context) {
    char pin_names[][4] = {"PC0", "PC1", "PC3", "PB2", "PB3", "PA4", "PA6", "PA7"};
    GpioPin gpio[] = {
        {.port = GPIOC, .pin = LL_GPIO_PIN_0},
        {.port = GPIOC, .pin = LL_GPIO_PIN_1},
        {.port = GPIOC, .pin = LL_GPIO_PIN_3},
        {.port = GPIOB, .pin = LL_GPIO_PIN_2},
        {.port = GPIOB, .pin = LL_GPIO_PIN_3},
        {.port = GPIOA, .pin = LL_GPIO_PIN_4},
        {.port = GPIOA, .pin = LL_GPIO_PIN_6},
        {.port = GPIOA, .pin = LL_GPIO_PIN_7}};
    uint8_t num = 0;
    bool pin_found = false;

    // Get first word as pin name
    string_t pin_name;
    string_init(pin_name);
    size_t ws = string_search_char(args, ' ');
    if(ws == STRING_FAILURE) {
        printf("Wrong input. Correct usage: gpio_set <pin_name> <0|1>");
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
        LL_GPIO_SetPinMode(gpio[num].port, gpio[num].pin, LL_GPIO_MODE_OUTPUT);
        LL_GPIO_SetPinOutputType(gpio[num].port, gpio[num].pin, LL_GPIO_OUTPUT_PUSHPULL);
        LL_GPIO_SetOutputPin(gpio[num].port, gpio[num].pin);
    } else {
        printf("Wrong 2nd argument. Use \"1\" to set, \"0\" to reset");
    }
    return;
}

void cli_command_os_info(string_t args, void* context) {
    const uint8_t threads_num_max = 32;
    osThreadId_t threads_id[threads_num_max];
    uint8_t thread_num = osThreadEnumerate(threads_id, threads_num_max);

    printf("Free HEAP size: %d\r\n", xPortGetFreeHeapSize());
    printf("Minimum heap size: %d\r\n", xPortGetMinimumEverFreeHeapSize());
    printf("%d threads in total:\r\n", thread_num);
    printf("%-20s %-14s %-14s %s\r\n", "Name", "Stack start", "Stack alloc", "Stack free");
    for(uint8_t i = 0; i < thread_num; i++) {
        TaskControlBlock* tcb = (TaskControlBlock*)threads_id[i];
        printf(
            "%-20s 0x%-12lx %-14ld %ld\r\n",
            osThreadGetName(threads_id[i]),
            (uint32_t)tcb->pxStack,
            (uint32_t)(tcb->pxEndOfStack - tcb->pxStack + 1) * sizeof(uint32_t),
            osThreadGetStackSpace(threads_id[i]) * sizeof(uint32_t));
    }
    return;
}

void cli_commands_init(Cli* cli) {
    cli_add_command(cli, "help", cli_command_help, cli);
    cli_add_command(cli, "?", cli_command_help, cli);
    cli_add_command(cli, "version", cli_command_version, cli);
    cli_add_command(cli, "!", cli_command_version, cli);
    cli_add_command(cli, "uid", cli_command_uuid, cli);
    cli_add_command(cli, "date", cli_command_date, cli);
    cli_add_command(cli, "log", cli_command_log, cli);
    cli_add_command(cli, "vibro", cli_command_vibro, cli);
    cli_add_command(cli, "led", cli_command_led, cli);
    cli_add_command(cli, "gpio_set", cli_command_gpio_set, cli);
    cli_add_command(cli, "os_info", cli_command_os_info, cli);
}
