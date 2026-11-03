/**
 * @file ra_raw_plugin.h
 * @brief App side of the Decode RAW plugin contract.
 *
 * A capture is not one signal: it is however many bursts the recording caught, and a remote
 * held down puts the same code in there dozens of times. So the plugin decodes the whole file
 * first, collects the distinct readings, and then shows them as a list to pick from - the same
 * shape the Sub-GHz app's Decode RAW has. All three of its screens live in the .fal, so none
 * of this is resident while the app is listening to the air.
 *
 * As with the other plugins, the .fal resolves its symbols from the firmware API table alone.
 * Everything it needs arrives in RaRawEnv, which the app owns.
 */
#pragma once

#include <gui/view_dispatcher.h>
#include <gui/icon.h>
#include <dialogs/dialogs.h>
#include <lib/subghz/receiver.h>

#define RA_RAW_PLUGIN_APP_ID      "RemoteAnalyzerDecodeRaw"
#define RA_RAW_PLUGIN_API_VERSION 1

/**
 * What @ref RaRawFormat reports in its kind out-parameter, which is also the order the icons
 * come in. It mirrors RaReadViewKind, and the scene asserts that it still does.
 */
typedef enum {
    RaRawKindNone = 0,
    RaRawKindUnknown,
    RaRawKindStatic,
    RaRawKindDynamic,
    RaRawKindCount,
} RaRawKind;

/** Turn a decoded frame into the text the live screen would show for it. */
typedef void (
    *RaRawFormat)(SubGhzProtocolDecoderBase* decoder_base, FuriString* text, uint8_t* kind);

/** Why the app is being asked to make a noise. */
typedef enum {
    RaRawNotifyFound, /**< the capture decoded into something */
    RaRawNotifyEmpty, /**< read through with nothing to show, or it would not open */
    RaRawNotifyNotRaw, /**< the pick was a saved key, not a capture */
} RaRawNotify;

/**
 * Called on the GUI thread, as it happens. A posted event would be no good for the pick: the
 * picker loop runs inside on_enter, so the app would not see it until the choosing was over.
 */
typedef void (*RaRawNotifyCallback)(void* context, RaRawNotify what);

typedef struct {
    /* In: what the decode needs to run */
    ViewDispatcher* view_dispatcher;
    uint32_t view_id_progress; /**< while the file is being read */
    uint32_t view_id_list; /**< what came out of it */
    uint32_t view_id_details; /**< one entry in full */
    uint32_t park_view_id; /**< app-owned view to park on before ours are removed */
    DialogsApp* dialogs;
    SubGhzReceiver* receiver; /**< the app's decoders, fed from the file instead of the air */
    RaRawFormat format;
    void* notify_context;
    RaRawNotifyCallback notify;
    const Icon* icon_file; /**< shown beside each capture in the picker */
    /** Indexed by RaRawKind. No I_* symbol reaches a plugin, so the app lends out the three
     * its own Read screen draws; the None slot is NULL. */
    const Icon* icons[RaRawKindCount];

    /* In: custom event ids to post, so the app keeps its own enum */
    uint32_t event_cancelled; /**< nothing was picked, so there is nothing to show */
} RaRawEnv;

/** What the .fal exports. */
typedef struct {
    /** Ask for a capture and start reading it. Posts event_cancelled if nothing is picked. */
    void (*on_enter)(RaRawEnv* env);
    /** Read one tick's worth of samples. False once there is no more work to do. */
    bool (*tick)(RaRawEnv* env);
    /** Park, unregister all three screens and free. */
    void (*on_exit)(RaRawEnv* env);
} RaRawPlugin;
