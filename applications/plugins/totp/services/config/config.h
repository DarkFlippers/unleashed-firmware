#pragma once

#include <flipper_format/flipper_format.h>
#include <furi.h>
#include "../../types/plugin_state.h"
#include "../../types/token_info.h"
#include "constants.h"

Storage* totp_open_storage();
void totp_close_storage();
FlipperFormat* totp_open_config_file(Storage* storage);
void totp_close_config_file(FlipperFormat* file);
void totp_full_save_config_file(PluginState* const plugin_state);
void totp_config_file_load_base(PluginState* const plugin_state);
void totp_config_file_load_tokens(PluginState* const plugin_state);
void totp_config_file_save_new_token(TokenInfo* token_info);
void totp_config_file_update_timezone_offset(float new_timezone_offset);
