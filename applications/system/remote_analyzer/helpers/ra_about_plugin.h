/**
 * @file ra_about_plugin.h
 * @brief App side of the How to use plugin contract.
 *
 * The manual is a page and a half of text and nothing else, which is exactly the kind of
 * weight worth keeping out of the app image until someone asks for it.
 *
 * As with the other plugins, the .fal resolves its symbols from the firmware API table alone.
 * Everything it needs arrives in RaAboutEnv, which the app owns.
 */
#pragma once

#include <gui/view_dispatcher.h>

#define RA_ABOUT_PLUGIN_APP_ID      "RemoteAnalyzerAbout"
#define RA_ABOUT_PLUGIN_API_VERSION 1

typedef struct {
    ViewDispatcher* view_dispatcher;
    uint32_t view_id; /**< id to register the manual's view under */
    uint32_t park_view_id; /**< app-owned view to park on before ours is removed */
} RaAboutEnv;

/** What the .fal exports. */
typedef struct {
    /** Allocate the widget, register it with the dispatcher and switch to it. */
    void (*on_enter)(RaAboutEnv* env);
    /** Park on park_view_id, unregister and free. */
    void (*on_exit)(RaAboutEnv* env);
} RaAboutPlugin;
