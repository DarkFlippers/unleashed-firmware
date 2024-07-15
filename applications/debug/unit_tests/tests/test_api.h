#pragma once

#include <flipper_application/flipper_application.h>

#define APPID       "UnitTest"
#define API_VERSION (0u)

typedef struct {
    int (*run)(void);
    int (*get_minunit_run)(void);
    int (*get_minunit_assert)(void);
    int (*get_minunit_status)(void);
} TestApi;

#define TEST_API_DEFINE(entrypoint)                     \
    const TestApi test_api = {                          \
        .run = entrypoint,                              \
        .get_minunit_run = get_minunit_run,             \
        .get_minunit_assert = get_minunit_assert,       \
        .get_minunit_status = get_minunit_status,       \
    };                                                  \
    const FlipperAppPluginDescriptor app_descriptor = { \
        .appid = APPID,                                 \
        .ep_api_version = API_VERSION,                  \
        .entry_point = &test_api,                       \
    };                                                  \
    const FlipperAppPluginDescriptor* get_api(void) {   \
        return &app_descriptor;                         \
    }
