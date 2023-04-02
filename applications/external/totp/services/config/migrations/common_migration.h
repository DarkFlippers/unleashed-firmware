#pragma once

#include <flipper_format/flipper_format.h>

bool totp_config_migrate_to_latest(
    FlipperFormat* fff_data_file,
    FlipperFormat* fff_backup_data_file);