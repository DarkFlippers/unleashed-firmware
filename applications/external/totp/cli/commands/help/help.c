#include "help.h"
#include "../../cli_helpers.h"
#include "../add/add.h"
#include "../update/update.h"
#include "../delete/delete.h"
#include "../list/list.h"
#include "../timezone/timezone.h"
#include "../move/move.h"
#include "../pin/pin.h"
#include "../notification/notification.h"
#include "../reset/reset.h"
#include "../automation/automation.h"
#include "../details/details.h"

#ifdef TOTP_CLI_RICH_HELP_ENABLED
void totp_cli_command_help_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_HELP ", " TOTP_CLI_COMMAND_HELP_ALT
                    ", " TOTP_CLI_COMMAND_HELP_ALT2 "       Show command usage help\r\n");
}

void totp_cli_command_help_docopt_usage() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_NAME " " DOCOPT_REQUIRED(
        TOTP_CLI_COMMAND_HELP " | " TOTP_CLI_COMMAND_HELP_ALT
                              " | " TOTP_CLI_COMMAND_HELP_ALT2) "\r\n");
}
#endif

void totp_cli_command_help_handle() {
#ifdef TOTP_CLI_RICH_HELP_ENABLED
    TOTP_CLI_PRINTF("Usage:\r\n");
    totp_cli_command_help_docopt_usage();
    totp_cli_command_list_docopt_usage();
    totp_cli_command_details_docopt_usage();
    totp_cli_command_add_docopt_usage();
    totp_cli_command_update_docopt_usage();
    totp_cli_command_delete_docopt_usage();
    totp_cli_command_timezone_docopt_usage();
    totp_cli_command_move_docopt_usage();
    totp_cli_command_pin_docopt_usage();
    totp_cli_command_notification_docopt_usage();
    totp_cli_command_reset_docopt_usage();
    totp_cli_command_automation_docopt_usage();
    cli_nl();
    TOTP_CLI_PRINTF("Commands:\r\n");
    totp_cli_command_help_docopt_commands();
    totp_cli_command_list_docopt_commands();
    totp_cli_command_details_docopt_commands();
    totp_cli_command_add_docopt_commands();
    totp_cli_command_update_docopt_commands();
    totp_cli_command_delete_docopt_commands();
    totp_cli_command_timezone_docopt_commands();
    totp_cli_command_move_docopt_commands();
    totp_cli_command_pin_docopt_commands();
    totp_cli_command_notification_docopt_commands();
    totp_cli_command_reset_docopt_commands();
    totp_cli_command_automation_docopt_commands();
    cli_nl();
    TOTP_CLI_PRINTF("Arguments:\r\n");
    totp_cli_command_add_docopt_arguments();
    totp_cli_command_delete_docopt_arguments();
    totp_cli_command_move_docopt_arguments();
    totp_cli_command_timezone_docopt_arguments();
    totp_cli_command_notification_docopt_arguments();
    totp_cli_command_automation_docopt_arguments();
    cli_nl();
    TOTP_CLI_PRINTF("Options:\r\n");
    totp_cli_command_add_docopt_options();
    totp_cli_command_update_docopt_options();
    totp_cli_command_delete_docopt_options();
    totp_cli_command_pin_docopt_options();
    totp_cli_command_automation_docopt_options();
#else
    TOTP_CLI_PRINTF(
        "All the TOTP CLI commands, their arguments, options and usage can be found here https://t.ly/_6pJG");
#endif
}