#include "cli_commands.h"
#include <api-hal.h>
#include <rtc.h>

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
    cli_print_version();
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

void cli_commands_init(Cli* cli) {
    cli_add_command(cli, "help", cli_command_help, cli);
    cli_add_command(cli, "?", cli_command_help, cli);
    cli_add_command(cli, "version", cli_command_version, cli);
    cli_add_command(cli, "!", cli_command_version, cli);
    cli_add_command(cli, "uid", cli_command_uuid, cli);
    cli_add_command(cli, "date", cli_command_date, cli);
    cli_add_command(cli, "log", cli_command_log, cli);
}
