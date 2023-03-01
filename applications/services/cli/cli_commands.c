#include "cli_commands.h"
#include "cli_command_gpio.h"

#include <furi_hal.h>
#include <furi_hal_info.h>
#include <task_control_block.h>
#include <time.h>
#include <notification/notification_messages.h>
#include <loader/loader.h>
#include <lib/toolbox/args.h>

// Close to ISO, `date +'%Y-%m-%d %H:%M:%S %u'`
#define CLI_DATE_FORMAT "%.4d-%.2d-%.2d %.2d:%.2d:%.2d %d"

void cli_command_info_callback(const char* key, const char* value, bool last, void* context) {
    UNUSED(last);
    UNUSED(context);
    printf("%-30s: %s\r\n", key, value);
}

/** Info Command
 *
 * This command is intended to be used by humans
 *
 * Arguments:
 * - device - print device info
 * - power - print power info
 * - power_debug - print power debug info
 *
 * @param      cli      The cli instance
 * @param      args     The arguments
 * @param      context  The context
 */
void cli_command_info(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);

    if(context) {
        furi_hal_info_get(cli_command_info_callback, '_', NULL);
        return;
    }

    if(!furi_string_cmp(args, "device")) {
        furi_hal_info_get(cli_command_info_callback, '.', NULL);
    } else if(!furi_string_cmp(args, "power")) {
        furi_hal_power_info_get(cli_command_info_callback, '.', NULL);
    } else if(!furi_string_cmp(args, "power_debug")) {
        furi_hal_power_debug_get(cli_command_info_callback, NULL);
    } else {
        cli_print_usage("info", "<device|power|power_debug>", furi_string_get_cstr(args));
    }
}

void cli_command_help(Cli* cli, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);
    printf("Commands available:");

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
            printf("%-30s", furi_string_get_cstr(*CliCommandTree_ref(it_left)->key_ptr));
            CliCommandTree_next(it_left);
        }
        // Right Column
        if(!CliCommandTree_end_p(it_right)) {
            printf("%s", furi_string_get_cstr(*CliCommandTree_ref(it_right)->key_ptr));
            CliCommandTree_next(it_right);
        }
    };

    if(furi_string_size(args) > 0) {
        cli_nl();
        printf("`");
        printf("%s", furi_string_get_cstr(args));
        printf("` command not found");
    }
}

void cli_command_date(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);

    FuriHalRtcDateTime datetime = {0};

    if(furi_string_size(args) > 0) {
        uint16_t hours, minutes, seconds, month, day, year, weekday;
        int ret = sscanf(
            furi_string_get_cstr(args),
            "%hu-%hu-%hu %hu:%hu:%hu %hu",
            &year,
            &month,
            &day,
            &hours,
            &minutes,
            &seconds,
            &weekday);

        // Some variables are going to discard upper byte
        // There will be some funky behaviour which is not breaking anything
        datetime.hour = hours;
        datetime.minute = minutes;
        datetime.second = seconds;
        datetime.weekday = weekday;
        datetime.month = month;
        datetime.day = day;
        datetime.year = year;

        if(ret != 7) {
            printf(
                "Invalid datetime format, use `%s`. sscanf %d %s",
                "%Y-%m-%d %H:%M:%S %u",
                ret,
                furi_string_get_cstr(args));
            return;
        }

        if(!furi_hal_rtc_validate_datetime(&datetime)) {
            printf("Invalid datetime data");
            return;
        }

        furi_hal_rtc_set_datetime(&datetime);
        // Verification
        furi_hal_rtc_get_datetime(&datetime);
        printf(
            "New datetime is: " CLI_DATE_FORMAT,
            datetime.year,
            datetime.month,
            datetime.day,
            datetime.hour,
            datetime.minute,
            datetime.second,
            datetime.weekday);
    } else {
        furi_hal_rtc_get_datetime(&datetime);
        printf(
            CLI_DATE_FORMAT,
            datetime.year,
            datetime.month,
            datetime.day,
            datetime.hour,
            datetime.minute,
            datetime.second,
            datetime.weekday);
    }
}

#define CLI_COMMAND_LOG_RING_SIZE 2048
#define CLI_COMMAND_LOG_BUFFER_SIZE 64

void cli_command_log_tx_callback(const uint8_t* buffer, size_t size, void* context) {
    furi_stream_buffer_send(context, buffer, size, 0);
}

void cli_command_log_level_set_from_string(FuriString* level) {
    if(furi_string_cmpi_str(level, "default") == 0) {
        furi_log_set_level(FuriLogLevelDefault);
    } else if(furi_string_cmpi_str(level, "none") == 0) {
        furi_log_set_level(FuriLogLevelNone);
    } else if(furi_string_cmpi_str(level, "error") == 0) {
        furi_log_set_level(FuriLogLevelError);
    } else if(furi_string_cmpi_str(level, "warn") == 0) {
        furi_log_set_level(FuriLogLevelWarn);
    } else if(furi_string_cmpi_str(level, "info") == 0) {
        furi_log_set_level(FuriLogLevelInfo);
    } else if(furi_string_cmpi_str(level, "debug") == 0) {
        furi_log_set_level(FuriLogLevelDebug);
    } else if(furi_string_cmpi_str(level, "trace") == 0) {
        furi_log_set_level(FuriLogLevelTrace);
    } else {
        printf("Unknown log level\r\n");
    }
}

void cli_command_log(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    FuriStreamBuffer* ring = furi_stream_buffer_alloc(CLI_COMMAND_LOG_RING_SIZE, 1);
    uint8_t buffer[CLI_COMMAND_LOG_BUFFER_SIZE];
    FuriLogLevel previous_level = furi_log_get_level();
    bool restore_log_level = false;

    if(furi_string_size(args) > 0) {
        cli_command_log_level_set_from_string(args);
        restore_log_level = true;
    }

    furi_hal_console_set_tx_callback(cli_command_log_tx_callback, ring);

    printf("Press CTRL+C to stop...\r\n");
    while(!cli_cmd_interrupt_received(cli)) {
        size_t ret = furi_stream_buffer_receive(ring, buffer, CLI_COMMAND_LOG_BUFFER_SIZE, 50);
        cli_write(cli, buffer, ret);
    }

    furi_hal_console_set_tx_callback(NULL, NULL);

    if(restore_log_level) {
        // There will be strange behaviour if log level is set from settings while log command is running
        furi_log_set_level(previous_level);
    }

    furi_stream_buffer_free(ring);
}

void cli_command_sysctl_debug(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    if(!furi_string_cmp(args, "0")) {
        furi_hal_rtc_reset_flag(FuriHalRtcFlagDebug);
        loader_update_menu();
        printf("Debug disabled.");
    } else if(!furi_string_cmp(args, "1")) {
        furi_hal_rtc_set_flag(FuriHalRtcFlagDebug);
        loader_update_menu();
        printf("Debug enabled.");
    } else {
        cli_print_usage("sysctl debug", "<1|0>", furi_string_get_cstr(args));
    }
}

void cli_command_sysctl_heap_track(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    if(!furi_string_cmp(args, "none")) {
        furi_hal_rtc_set_heap_track_mode(FuriHalRtcHeapTrackModeNone);
        printf("Heap tracking disabled");
    } else if(!furi_string_cmp(args, "main")) {
        furi_hal_rtc_set_heap_track_mode(FuriHalRtcHeapTrackModeMain);
        printf("Heap tracking enabled for application main thread");
#if FURI_DEBUG
    } else if(!furi_string_cmp(args, "tree")) {
        furi_hal_rtc_set_heap_track_mode(FuriHalRtcHeapTrackModeTree);
        printf("Heap tracking enabled for application main and child threads");
    } else if(!furi_string_cmp(args, "all")) {
        furi_hal_rtc_set_heap_track_mode(FuriHalRtcHeapTrackModeAll);
        printf("Heap tracking enabled for all threads");
#endif
    } else {
        cli_print_usage("sysctl heap_track", "<none|main|tree|all>", furi_string_get_cstr(args));
    }
}

void cli_command_sysctl_print_usage() {
    printf("Usage:\r\n");
    printf("sysctl <cmd> <args>\r\n");
    printf("Cmd list:\r\n");

    printf("\tdebug <0|1>\t - Enable or disable system debug\r\n");
#if FURI_DEBUG
    printf("\theap_track <none|main|tree|all>\t - Set heap allocation tracking mode\r\n");
#else
    printf("\theap_track <none|main>\t - Set heap allocation tracking mode\r\n");
#endif
}

void cli_command_sysctl(Cli* cli, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            cli_command_sysctl_print_usage();
            break;
        }

        if(furi_string_cmp_str(cmd, "debug") == 0) {
            cli_command_sysctl_debug(cli, args, context);
            break;
        }

        if(furi_string_cmp_str(cmd, "heap_track") == 0) {
            cli_command_sysctl_heap_track(cli, args, context);
            break;
        }

        cli_command_sysctl_print_usage();
    } while(false);

    furi_string_free(cmd);
}

void cli_command_vibro(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    if(!furi_string_cmp(args, "0")) {
        NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
        notification_message_block(notification, &sequence_reset_vibro);
        furi_record_close(RECORD_NOTIFICATION);
    } else if(!furi_string_cmp(args, "1")) {
        NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
        notification_message_block(notification, &sequence_set_vibro_on);
        furi_record_close(RECORD_NOTIFICATION);
    } else {
        cli_print_usage("vibro", "<1|0>", furi_string_get_cstr(args));
    }
}

void cli_command_led(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(context);
    // Get first word as light name
    NotificationMessage notification_led_message;
    FuriString* light_name;
    light_name = furi_string_alloc();
    size_t ws = furi_string_search_char(args, ' ');
    if(ws == FURI_STRING_FAILURE) {
        cli_print_usage("led", "<r|g|b|bl> <0-255>", furi_string_get_cstr(args));
        furi_string_free(light_name);
        return;
    } else {
        furi_string_set_n(light_name, args, 0, ws);
        furi_string_right(args, ws);
        furi_string_trim(args);
    }
    // Check light name
    if(!furi_string_cmp(light_name, "r")) {
        notification_led_message.type = NotificationMessageTypeLedRed;
    } else if(!furi_string_cmp(light_name, "g")) {
        notification_led_message.type = NotificationMessageTypeLedGreen;
    } else if(!furi_string_cmp(light_name, "b")) {
        notification_led_message.type = NotificationMessageTypeLedBlue;
    } else if(!furi_string_cmp(light_name, "bl")) {
        notification_led_message.type = NotificationMessageTypeLedDisplayBacklight;
    } else {
        cli_print_usage("led", "<r|g|b|bl> <0-255>", furi_string_get_cstr(args));
        furi_string_free(light_name);
        return;
    }
    furi_string_free(light_name);
    // Read light value from the rest of the string
    char* end_ptr;
    uint32_t value = strtoul(furi_string_get_cstr(args), &end_ptr, 0);
    if(!(value < 256 && *end_ptr == '\0')) {
        cli_print_usage("led", "<r|g|b|bl> <0-255>", furi_string_get_cstr(args));
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
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_internal_message_block(notification, &notification_sequence);
    furi_record_close(RECORD_NOTIFICATION);
}

void cli_command_ps(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);

    const uint8_t threads_num_max = 32;
    FuriThreadId threads_ids[threads_num_max];
    uint8_t thread_num = furi_thread_enumerate(threads_ids, threads_num_max);
    printf(
        "%-20s %-20s %-14s %-8s %-8s %s\r\n",
        "AppID",
        "Name",
        "Stack start",
        "Heap",
        "Stack",
        "Stack min free");
    for(uint8_t i = 0; i < thread_num; i++) {
        TaskControlBlock* tcb = (TaskControlBlock*)threads_ids[i];
        printf(
            "%-20s %-20s 0x%-12lx %-8zu %-8lu %-8lu\r\n",
            furi_thread_get_appid(threads_ids[i]),
            furi_thread_get_name(threads_ids[i]),
            (uint32_t)tcb->pxStack,
            memmgr_heap_get_thread_memory(threads_ids[i]),
            (uint32_t)(tcb->pxEndOfStack - tcb->pxStack + 1) * sizeof(StackType_t),
            furi_thread_get_stack_space(threads_ids[i]));
    }
    printf("\r\nTotal: %d", thread_num);
}

void cli_command_free(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);

    printf("Free heap size: %zu\r\n", memmgr_get_free_heap());
    printf("Total heap size: %zu\r\n", memmgr_get_total_heap());
    printf("Minimum heap size: %zu\r\n", memmgr_get_minimum_free_heap());
    printf("Maximum heap block: %zu\r\n", memmgr_heap_get_max_free_block());

    printf("Pool free: %zu\r\n", memmgr_pool_get_free());
    printf("Maximum pool block: %zu\r\n", memmgr_pool_get_max_block());
}

void cli_command_free_blocks(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);

    memmgr_heap_printf_free_blocks();
}

void cli_command_i2c(Cli* cli, FuriString* args, void* context) {
    UNUSED(cli);
    UNUSED(args);
    UNUSED(context);

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    printf("Scanning external i2c on PC0(SCL)/PC1(SDA)\r\n"
           "Clock: 100khz, 7bit address\r\n"
           "\r\n");
    printf("  | 0 1 2 3 4 5 6 7 8 9 A B C D E F\r\n");
    printf("--+--------------------------------\r\n");
    for(uint8_t row = 0; row < 0x8; row++) {
        printf("%x | ", row);
        for(uint8_t column = 0; column <= 0xF; column++) {
            bool ret = furi_hal_i2c_is_device_ready(
                &furi_hal_i2c_handle_external, ((row << 4) + column) << 1, 2);
            printf("%c ", ret ? '#' : '-');
        }
        printf("\r\n");
    }
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);
}

void cli_commands_init(Cli* cli) {
    cli_add_command(cli, "!", CliCommandFlagParallelSafe, cli_command_info, (void*)true);
    cli_add_command(cli, "info", CliCommandFlagParallelSafe, cli_command_info, NULL);
    cli_add_command(cli, "device_info", CliCommandFlagParallelSafe, cli_command_info, (void*)true);

    cli_add_command(cli, "?", CliCommandFlagParallelSafe, cli_command_help, NULL);
    cli_add_command(cli, "help", CliCommandFlagParallelSafe, cli_command_help, NULL);

    cli_add_command(cli, "date", CliCommandFlagParallelSafe, cli_command_date, NULL);
    cli_add_command(cli, "log", CliCommandFlagParallelSafe, cli_command_log, NULL);
    cli_add_command(cli, "sysctl", CliCommandFlagDefault, cli_command_sysctl, NULL);
    cli_add_command(cli, "ps", CliCommandFlagParallelSafe, cli_command_ps, NULL);
    cli_add_command(cli, "free", CliCommandFlagParallelSafe, cli_command_free, NULL);
    cli_add_command(cli, "free_blocks", CliCommandFlagParallelSafe, cli_command_free_blocks, NULL);

    cli_add_command(cli, "vibro", CliCommandFlagDefault, cli_command_vibro, NULL);
    cli_add_command(cli, "led", CliCommandFlagDefault, cli_command_led, NULL);
    cli_add_command(cli, "gpio", CliCommandFlagDefault, cli_command_gpio, NULL);
    cli_add_command(cli, "i2c", CliCommandFlagDefault, cli_command_i2c, NULL);
}
