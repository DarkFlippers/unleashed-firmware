#pragma once
#include "defines.h"
#define CONFIG_FILE_HEADER "Blackjack config file"
#define CONFIG_FILE_VERSION 1

void save_settings(Settings settings);
Settings load_settings();