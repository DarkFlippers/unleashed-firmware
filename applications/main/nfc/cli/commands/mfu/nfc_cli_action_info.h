#pragma once

#include "../../nfc_cli_command_base_i.h"

NfcCliActionContext* nfc_cli_mfu_info_alloc_ctx(Nfc* nfc);
void nfc_cli_mfu_info_free_ctx(NfcCliActionContext* ctx);
void nfc_cli_mfu_info_execute(PipeSide* pipe, NfcCliActionContext* ctx);
