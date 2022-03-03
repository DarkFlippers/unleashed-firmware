#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <lib/toolbox/level_duration.h>

typedef struct {
    bool is_runing;
    size_t repeat;
    size_t front;
    size_t size_upload;
    LevelDuration* upload;

} SubGhzProtocolBlockEncoder;
