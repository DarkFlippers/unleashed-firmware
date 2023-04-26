#pragma once

#include <flipper_format/flipper_format.h>

/**
 * @brief Migrates config file to the latest version
 * @param fff_data_file original config file to be migrated
 * @param fff_backup_data_file backup copy of original config file
 * @return \c true if operation succeeded; \c false otherwise
 */
bool totp_config_migrate_to_latest(
    FlipperFormat* fff_data_file,
    FlipperFormat* fff_backup_data_file);
