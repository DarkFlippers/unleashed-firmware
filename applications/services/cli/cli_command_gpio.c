#include "cli_command_gpio.h"

#include "core/string.h"
#include <furi.h>
#include <furi_hal.h>
#include <lib/toolbox/args.h>

typedef struct {
    const GpioPin* pin;
    const char* name;
    const bool debug;
} CliCommandGpio;

const CliCommandGpio cli_command_gpio_pins[] = {
    {.pin = &gpio_ext_pc0, .name = "PC0", .debug = false},
    {.pin = &gpio_ext_pc1, .name = "PC1", .debug = false},
    {.pin = &gpio_ext_pc3, .name = "PC3", .debug = false},
    {.pin = &gpio_ext_pb2, .name = "PB2", .debug = false},
    {.pin = &gpio_ext_pb3, .name = "PB3", .debug = false},
    {.pin = &gpio_ext_pa4, .name = "PA4", .debug = false},
    {.pin = &gpio_ext_pa6, .name = "PA6", .debug = false},
    {.pin = &gpio_ext_pa7, .name = "PA7", .debug = false},
    /* Dangerous pins, may damage hardware */
    {.pin = &gpio_infrared_rx, .name = "PA0", .debug = true},
    {.pin = &gpio_usart_rx, .name = "PB7", .debug = true},
    {.pin = &gpio_speaker, .name = "PB8", .debug = true},
    {.pin = &gpio_infrared_tx, .name = "PB9", .debug = true},
};

void cli_command_gpio_print_usage() {
    printf("Usage:\r\n");
    printf("gpio <cmd> <args>\r\n");
    printf("Cmd list:\r\n");
    printf("\tmode <pin_name> <0|1>\t - Set gpio mode: 0 - input, 1 - output\r\n");
    printf("\tset <pin_name> <0|1>\t - Set gpio value\r\n");
    printf("\tread <pin_name>\t - Read gpio value\r\n");
}

static bool pin_name_to_int(FuriString* pin_name, size_t* result) {
    bool is_debug_mode = furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug);
    for(size_t i = 0; i < COUNT_OF(cli_command_gpio_pins); i++) {
        if(furi_string_equal(pin_name, cli_command_gpio_pins[i].name)) {
            if(!cli_command_gpio_pins[i].debug || is_debug_mode) {
                *result = i;
                return true;
            }
        }
    }

    return false;
}

static void gpio_print_pins(void) {
    printf("Wrong pin name. Available pins: ");
    bool is_debug_mode = furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug);
    for(size_t i = 0; i < COUNT_OF(cli_command_gpio_pins); i++) {
        if(!cli_command_gpio_pins[i].debug || is_debug_mode) {
            printf("%s ", cli_command_gpio_pins[i].name);
        }
    }
}

typedef enum {
    GpioParseReturnOk,
    GpioParseReturnCmdSyntaxError,
    GpioParseReturnPinError,
    GpioParseReturnValueError
} GpioParseReturn;

static GpioParseReturn gpio_command_parse(FuriString* args, size_t* pin_num, uint8_t* value) {
    GpioParseReturn ret = GpioParseReturnOk;
    FuriString* pin_name = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, pin_name)) {
            ret = GpioParseReturnCmdSyntaxError;
            break;
        } else if(!pin_name_to_int(pin_name, pin_num)) {
            ret = GpioParseReturnPinError;
            break;
        }

        int pin_mode; //-V779
        if(!args_read_int_and_trim(args, &pin_mode) || pin_mode < 0 || pin_mode > 1) {
            ret = GpioParseReturnValueError;
            break;
        }

        *value = pin_mode;
    } while(false);

    furi_string_free(pin_name);
    return ret;
}

void cli_command_gpio_mode(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);

    size_t num = 0;
    uint8_t value = 255;

    GpioParseReturn err = gpio_command_parse(args, &num, &value);

    if(err == GpioParseReturnCmdSyntaxError) {
        cli_print_usage("gpio mode", "<pin_name> <0|1>", furi_string_get_cstr(args));
        return;
    } else if(err == GpioParseReturnPinError) { //-V547
        gpio_print_pins();
        return;
    } else if(err == GpioParseReturnValueError) {
        printf("Value is invalid. Enter 1 for input or 0 for output");
        return;
    }

    if(cli_command_gpio_pins[num].debug) { //-V779
        printf(
            "Changing this pin mode may damage hardware. Are you sure you want to continue? (y/n)?\r\n");
        char c = cli_getc(cli);
        if(c != 'y' && c != 'Y') {
            printf("Cancelled.\r\n");
            return;
        }
    }

    if(value == 1) { // output
        furi_hal_gpio_write(cli_command_gpio_pins[num].pin, false);
        furi_hal_gpio_init_simple(cli_command_gpio_pins[num].pin, GpioModeOutputPushPull);
        printf("Pin %s is now an output (low)", cli_command_gpio_pins[num].name);
    } else { // input
        furi_hal_gpio_init_simple(cli_command_gpio_pins[num].pin, GpioModeInput);
        printf("Pin %s is now an input", cli_command_gpio_pins[num].name);
    }
}

void cli_command_gpio_read(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);

    size_t num = 0;
    if(!pin_name_to_int(args, &num)) {
        gpio_print_pins();
        return;
    }

    if(LL_GPIO_MODE_INPUT != //-V779
       LL_GPIO_GetPinMode(
           cli_command_gpio_pins[num].pin->port, cli_command_gpio_pins[num].pin->pin)) {
        printf("Err: pin %s is not set as an input.", cli_command_gpio_pins[num].name);
        return;
    }

    uint8_t val = !!furi_hal_gpio_read(cli_command_gpio_pins[num].pin);

    printf("Pin %s <= %u", cli_command_gpio_pins[num].name, val);
}

void cli_command_gpio_set(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);

    size_t num = 0;
    uint8_t value = 0;
    GpioParseReturn err = gpio_command_parse(args, &num, &value);

    if(err == GpioParseReturnCmdSyntaxError) {
        cli_print_usage("gpio set", "<pin_name> <0|1>", furi_string_get_cstr(args));
        return;
    } else if(err == GpioParseReturnPinError) { //-V547
        gpio_print_pins();
        return;
    } else if(err == GpioParseReturnValueError) {
        printf("Value is invalid. Enter 1 for high or 0 for low");
        return;
    }

    if(LL_GPIO_MODE_OUTPUT != //-V779
       LL_GPIO_GetPinMode(
           cli_command_gpio_pins[num].pin->port, cli_command_gpio_pins[num].pin->pin)) {
        printf("Err: pin %s is not set as an output.", cli_command_gpio_pins[num].name);
        return;
    }

    // Extra check if debug pins used
    if(cli_command_gpio_pins[num].debug) {
        printf(
            "Setting this pin may damage hardware. Are you sure you want to continue? (y/n)?\r\n");
        char c = cli_getc(cli);
        if(c != 'y' && c != 'Y') {
            printf("Cancelled.\r\n");
            return;
        }
    }

    furi_hal_gpio_write(cli_command_gpio_pins[num].pin, !!value);
    printf("Pin %s => %u", cli_command_gpio_pins[num].name, !!value);
}

void cli_command_gpio(Cli* cli, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            cli_command_gpio_print_usage();
            break;
        }

        if(furi_string_cmp_str(cmd, "mode") == 0) {
            cli_command_gpio_mode(cli, args, context);
            break;
        }

        if(furi_string_cmp_str(cmd, "set") == 0) {
            cli_command_gpio_set(cli, args, context);
            break;
        }

        if(furi_string_cmp_str(cmd, "read") == 0) {
            cli_command_gpio_read(cli, args, context);
            break;
        }

        cli_command_gpio_print_usage();
    } while(false);

    furi_string_free(cmd);
}
