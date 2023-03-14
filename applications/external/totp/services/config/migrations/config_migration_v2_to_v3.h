#pragma once

#include <flipper_format/flipper_format.h>

bool totp_config_migrate_v2_to_v3(
    FlipperFormat* fff_data_file,
    FlipperFormat* fff_backup_data_file);
