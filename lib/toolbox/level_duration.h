#pragma once

#include <stdint.h>

#define LEVEL_DURATION_BIG

#ifdef LEVEL_DURATION_BIG

#define LEVEL_DURATION_RESET      0U
#define LEVEL_DURATION_LEVEL_LOW  1U
#define LEVEL_DURATION_LEVEL_HIGH 2U
#define LEVEL_DURATION_WAIT       3U
#define LEVEL_DURATION_RESERVED   0x800000U

typedef struct {
    uint32_t duration : 30;
    uint8_t level     : 2;
} LevelDuration;

static inline LevelDuration level_duration_make(bool level, uint32_t duration) {
    LevelDuration level_duration;
    level_duration.level = level ? LEVEL_DURATION_LEVEL_HIGH : LEVEL_DURATION_LEVEL_LOW;
    level_duration.duration = duration;
    return level_duration;
}

static inline LevelDuration level_duration_reset() {
    LevelDuration level_duration;
    level_duration.level = LEVEL_DURATION_RESET;
    return level_duration;
}

static inline LevelDuration level_duration_wait() {
    LevelDuration level_duration;
    level_duration.level = LEVEL_DURATION_WAIT;
    return level_duration;
}

static inline bool level_duration_is_reset(LevelDuration level_duration) {
    return level_duration.level == LEVEL_DURATION_RESET;
}

static inline bool level_duration_is_wait(LevelDuration level_duration) {
    return level_duration.level == LEVEL_DURATION_WAIT;
}

static inline bool level_duration_get_level(LevelDuration level_duration) {
    return level_duration.level == LEVEL_DURATION_LEVEL_HIGH;
}

static inline uint32_t level_duration_get_duration(LevelDuration level_duration) {
    return level_duration.duration;
}

#else

#define LEVEL_DURATION_RESET    0U
#define LEVEL_DURATION_RESERVED 0x800000U

typedef int32_t LevelDuration;

static inline LevelDuration level_duration(bool level, uint32_t duration) {
    return level ? duration : -(int32_t)duration;
}

static inline LevelDuration level_duration_reset() {
    return LEVEL_DURATION_RESET;
}

static inline bool level_duration_is_reset(LevelDuration level_duration) {
    return level_duration == LEVEL_DURATION_RESET;
}

static inline bool level_duration_get_level(LevelDuration level_duration) {
    return level_duration > 0;
}

static inline uint32_t level_duration_get_duration(LevelDuration level_duration) {
    return (level_duration >= 0) ? level_duration : -level_duration;
}

#endif
