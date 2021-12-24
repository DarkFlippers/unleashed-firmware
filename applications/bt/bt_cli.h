#pragma once

#include <cli/cli.h>

void bt_on_system_start();

void bt_cli_command_info(Cli* cli, string_t args, void* context);

void bt_cli_command_carrier_tx(Cli* cli, string_t args, void* context);

void bt_cli_command_carrier_rx(Cli* cli, string_t args, void* context);

void bt_cli_command_packet_tx(Cli* cli, string_t args, void* context);

void bt_cli_command_packet_rx(Cli* cli, string_t args, void* context);
