/**
 * @file cli_vcp.h
 * VCP HAL API
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_CLI_VCP "cli_vcp"

typedef struct CliVcp CliVcp;

void cli_vcp_enable(CliVcp* cli_vcp);
void cli_vcp_disable(CliVcp* cli_vcp);

#ifdef __cplusplus
}
#endif
