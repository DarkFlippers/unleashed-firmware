#include "help.h"
#include "../../cli_helpers.h"
#include "../add/add.h"
#include "../delete/delete.h"
#include "../list/list.h"
#include "../timezone/timezone.h"
#include "../move/move.h"
#include "../pin/pin.h"
#include "../notification/notification.h"

void totp_cli_command_help_docopt_commands() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_HELP ", " TOTP_CLI_COMMAND_HELP_ALT
                    ", " TOTP_CLI_COMMAND_HELP_ALT2 "       Show command usage help\r\n");
}

void totp_cli_command_help_docopt_usage() {
    TOTP_CLI_PRINTF("  " TOTP_CLI_COMMAND_NAME " " DOCOPT_REQUIRED(
        TOTP_CLI_COMMAND_HELP " | " TOTP_CLI_COMMAND_HELP_ALT
                              " | " TOTP_CLI_COMMAND_HELP_ALT2) "\r\n");
}

void totp_cli_command_help_handle() {
    TOTP_CLI_PRINTF("Usage:\r\n");
    totp_cli_command_help_docopt_usage();
    totp_cli_command_list_docopt_usage();
    totp_cli_command_add_docopt_usage();
    totp_cli_command_delete_docopt_usage();
    totp_cli_command_timezone_docopt_usage();
    totp_cli_command_move_docopt_usage();
    totp_cli_command_pin_docopt_usage();
    totp_cli_command_notification_docopt_usage();
    cli_nl();
    TOTP_CLI_PRINTF("Commands:\r\n");
    totp_cli_command_help_docopt_commands();
    totp_cli_command_list_docopt_commands();
    totp_cli_command_add_docopt_commands();
    totp_cli_command_delete_docopt_commands();
    totp_cli_command_timezone_docopt_commands();
    totp_cli_command_move_docopt_commands();
    totp_cli_command_pin_docopt_commands();
    totp_cli_command_notification_docopt_commands();
    cli_nl();
    TOTP_CLI_PRINTF("Arguments:\r\n");
    totp_cli_command_add_docopt_arguments();
    totp_cli_command_delete_docopt_arguments();
    totp_cli_command_timezone_docopt_arguments();
    totp_cli_command_notification_docopt_arguments();
    cli_nl();
    TOTP_CLI_PRINTF("Options:\r\n");
    totp_cli_command_add_docopt_options();
    totp_cli_command_delete_docopt_options();
    totp_cli_command_move_docopt_options();
}