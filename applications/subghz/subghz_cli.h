#pragma once

#include <cli/cli.h>

void subghz_cli_init();

void subghz_cli_command_tx_carrier(Cli* cli, string_t args, void* context);

void subghz_cli_command_rx_carrier(Cli* cli, string_t args, void* context);

void subghz_cli_command_tx_pt(Cli* cli, string_t args, void* context);

void subghz_cli_command_rx_pt(Cli* cli, string_t args, void* context);
