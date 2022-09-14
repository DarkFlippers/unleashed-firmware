#pragma once

#include "bt_i.h"
#include "bt_keys_filename.h"

bool bt_keys_storage_load(Bt* bt);

bool bt_keys_storage_save(Bt* bt);

bool bt_keys_storage_delete(Bt* bt);
