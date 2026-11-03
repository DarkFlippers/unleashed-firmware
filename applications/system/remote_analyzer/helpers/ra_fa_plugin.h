/**
 * @file ra_fa_plugin.h
 * @brief App side of the Frequency Analyzer plugin contract.
 *
 * The analyzer ships as a .fal embedded in the .fap: it is mapped when its scene is entered
 * and unmapped when the scene is left, so its code is only resident while it is on screen.
 *
 * The plugin resolves its symbols from the firmware API table alone - the app exports no API
 * of its own, which would mean a second resolver and a symbol table in flash. Everything it
 * needs arrives in RaFaEnv instead, and everything it has to report back goes into the same
 * struct, which the app owns.
 */
#pragma once

#include <gui/view_dispatcher.h>
#include <lib/subghz/subghz_setting.h>

#define RA_FA_PLUGIN_APP_ID      "RemoteAnalyzerFreqAnalyzer"
#define RA_FA_PLUGIN_API_VERSION 1

typedef enum {
    RaFaFeedbackAll, /**< sound and vibro on lock */
    RaFaFeedbackVibro, /**< vibro only */
    RaFaFeedbackMute,
} RaFaFeedback;

typedef struct {
    /* In: what the analyzer needs to run */
    ViewDispatcher* view_dispatcher;
    uint32_t view_id; /**< id to register the analyzer view under */
    uint32_t park_view_id; /**< app-owned view to park on before ours is removed */
    SubGhzSetting* setting; /**< frequency table to scan */

    /* In: custom event ids to post, so the app keeps its own enum */
    uint32_t event_lock;
    uint32_t event_unlock;
    uint32_t event_picked; /**< OK short: a frequency was picked */
    uint32_t event_go_read; /**< OK long: picked, and go to Read */

    /* In/out: carried across sessions by the app's settings file */
    float trigger;
    uint32_t feedback; /**< RaFaFeedback */

    /* Out */
    uint32_t frequency; /**< the picked frequency, 0 if none */
} RaFaEnv;

/** What the .fal exports. */
typedef struct {
    /** Allocate the view, register it with the dispatcher and switch to it. Starts the worker. */
    void (*on_enter)(RaFaEnv* env);
    /** Park on park_view_id, unregister, stop the worker and free. Writes settings back to env. */
    void (*on_exit)(RaFaEnv* env);
} RaFaPlugin;
