/**
 * @file remote_analyzer_i.h
 * @brief App state shared by the scenes.
 */
#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <notification/notification_messages.h>
#include <dialogs/dialogs.h>

#include "helpers/ra_settings.h"
#include "helpers/ra_radio.h"
#include "helpers/ra_sweep.h"
#include "helpers/ra_plugin_loader.h"
#include "helpers/ra_fa_plugin.h"
#include "helpers/ra_raw_plugin.h"
#include "helpers/ra_about_plugin.h"
#include "views/ra_read_view.h"
#include "scenes/ra_scene.h"

typedef enum {
    RaViewIdSubmenu,
    RaViewIdRead,
    RaViewIdVarItemList,
    /** Taken by whichever plugin is mapped, and given back on its way out. The analyzer and
     * the manual use the first; a Decode RAW has three screens and takes all of them. */
    RaViewIdPlugin,
    RaViewIdPluginList,
    RaViewIdPluginDetails,
} RaViewId;

typedef enum {
    RaCustomEventSignal = 100, /**< a protocol decoded a frame off the air */
    RaCustomEventReadConfig, /**< left on the Read screen */
    RaCustomEventReadReset, /**< right on the Read screen */
    RaCustomEventTick, /**< the Read screen's RSSI and sweep, and the file pump */
    RaCustomEventConfigReset, /**< "Reset to default" in the config */
    RaCustomEventPluginMissing, /**< a .fal would not load */
    RaCustomEventFaLock,
    RaCustomEventFaUnlock,
    RaCustomEventFaPicked,
    RaCustomEventFaGoRead,
    RaCustomEventRawCancelled, /**< no capture was picked */
} RaCustomEvent;

/** What the LED is saying. All three blink; the green is held briefly after a decode. */
typedef enum {
    RaLedStateNone = 0,
    RaLedStateReceiving, /**< purple: listening, nothing on the air */
    RaLedStateSignal, /**< yellow: a carrier is up that nothing has decoded */
    RaLedStateDecoded, /**< green: a frame came out */
} RaLedState;

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    NotificationApp* notifications;
    DialogsApp* dialogs;

    Submenu* submenu;
    VariableItemList* var_item_list;
    RaReadView* read_view;

    RaRadio* radio;
    RaSettings settings;

    /** What RX was last started with. Re-entering Read compares these against the settings
     * to tell whether the reading on screen still belongs to the current frequency. */
    uint32_t tuned_frequency;
    uint32_t tuned_preset_index;

    /** The last frame's description and kind. Written on the worker thread, read on the GUI
     * thread once the custom event lands, so they need the mutex. */
    FuriString* signal_text;
    uint8_t signal_kind; // RaReadViewKind
    /** The last code's text, and whether a code unlike it has arrived since the screen last
     * looked. A remote held down sends the same frame over and over, which is not news. */
    FuriString* signal_prev;
    bool signal_fresh;
    FuriMutex* signal_mutex;

    RaSweep sweep;
    /** Drives the Read screen's meter and sweep, and the file pump behind Decode RAW. */
    FuriTimer* tick_timer;

    uint8_t led_state; // RaLedState
    uint32_t led_green_from; // kernel ticks, when the last frame decoded
    bool led_green_hold;

    /** One .fal is mapped at a time, by whichever of the plugin scenes is up. */
    RaPluginLoader* plugin_loader;
    const RaFaPlugin* fa_plugin;
    RaFaEnv fa_env;
    const RaRawPlugin* raw_plugin;
    RaRawEnv raw_env;
    const RaAboutPlugin* about_plugin;
    RaAboutEnv about_env;
} RemoteAnalyzer;

/** Vibro and a chirp, for a code that is not the one already on screen. */
extern const NotificationSequence ra_sequence_new_signal;

/** Forget the last code, so the same one arriving next counts as a new signal again. */
void remote_analyzer_forget_last_signal(RemoteAnalyzer* app);

/**
 * Put the decoders back on the app's own callback, after a plugin has had them reporting to
 * itself. Has to run before that plugin's code is unmapped.
 */
void remote_analyzer_restore_rx_callback(RemoteAnalyzer* app);

/** The text and badge the Read screen shows for @p decoder_base. @ref RaRawFormat. */
void remote_analyzer_format_signal(
    SubGhzProtocolDecoderBase* decoder_base,
    FuriString* text,
    uint8_t* kind);
