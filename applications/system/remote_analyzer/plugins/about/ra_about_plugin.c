/**
 * @file ra_about_plugin.c
 * @brief Entry point of the How to use .fal: the manual, and the widget it scrolls in.
 */
#include "../../helpers/ra_about_plugin.h"

#include <flipper_application/flipper_application.h>
#include <gui/modules/widget.h>
#include <furi.h>

/* The scroll element only reads a control marker at the start of a line, so each heading
 * carries one \e# and no closing marker, and the next \n ends the bold. It wraps per
 * character rather than per word, so the breaks are placed here: every line is inside the
 * 124px the element leaves beside its scrollbar. */
/* clang-format off */
static const char* ra_about_text =
    "\e#Remote Analyzer\n"
    "Decodes one remote at a time\n"
    "on the internal radio and\n"
    "shows every field it can\n"
    "work out. One signal at a\n"
    "time, no history.\n"
    "\n"
    "\e#Extra protocols\n"
    "These the app decodes on\n"
    "its own, on top of every\n"
    "protocol the Sub-GHz app\n"
    "already knows:\n"
    "Crypt, Crypt Lite,\n"
    "Hormann BiSecur, KCE6xxx,\n"
    "Life Fido, Nero Intro II,\n"
    "PC4.\n"
    "\n"
    "\e#Read\n"
    "Pick the frequency and the\n"
    "modulation under Config,\n"
    "then hold the remote close\n"
    "and press its button.\n"
    "The icon at the top left\n"
    "says fixed or rolling code.\n"
    "The bar under the line is\n"
    "signal strength.\n"
    "Up and Down scroll a\n"
    "reading that does not fit.\n"
    "\n"
    "\e#The LED\n"
    "Purple: listening.\n"
    "Yellow: something is on the\n"
    "air that nothing decoded.\n"
    "Green: a code came through.\n"
    "\n"
    "\e#Notify\n"
    "Buzzes once on a code that\n"
    "differs from the last one.\n"
    "A remote held down repeats\n"
    "one code, so that only\n"
    "blinks the LED.\n"
    "\n"
    "\e#Mod. sweep\n"
    "When a remote is clearly\n"
    "transmitting but nothing\n"
    "decodes it, this walks the\n"
    "other modulations a second\n"
    "at a time. A ~ beside the\n"
    "modulation means it is\n"
    "still trying one. Keep the\n"
    "button held down while it\n"
    "does.\n"
    "The first code that decodes\n"
    "fixes the modulation, and\n"
    "the ~ goes away. It stays\n"
    "there while codes keep\n"
    "arriving, and the sweep\n"
    "starts again from it once\n"
    "two seconds pass with\n"
    "nothing decoding.\n"
    "Unknown remote means the\n"
    "whole round found nothing;\n"
    "it goes round again while\n"
    "the button is held.\n"
    "\n"
    "\e#Hopping\n"
    "OFF, or the level a carrier\n"
    "has to reach. On a level it\n"
    "walks 315, 390, 430.5,\n"
    "433.92, 434.42 and 868.35\n"
    "while the air is quiet and\n"
    "stops on the first band it\n"
    "hears something on.\n"
    "A decode holds the band for\n"
    "two seconds before it goes\n"
    "on hopping.\n"
    "The frequency in Config is\n"
    "ignored while this is on,\n"
    "and picking one in the\n"
    "Frequency Analyzer turns it\n"
    "off.\n"
    "The level is also what the\n"
    "sweep and the LED count as\n"
    "a signal, so it still\n"
    "applies with hopping OFF.\n"
    "\n"
    "\e#Decode RAW File\n"
    "Runs a saved .sub capture\n"
    "through the same decoders.\n"
    "The bar shows how far\n"
    "through the file it is,\n"
    "then every distinct code\n"
    "it found is listed. Press\n"
    "OK on one for the full\n"
    "reading.\n"
    "Only a RAW capture holds\n"
    "samples, so a saved key\n"
    "file is turned away.\n"
    "Nothing decoded, or a file\n"
    "turned away, beeps an error\n"
    "and holds the LED red.\n"
    "\n"
    "\e#Frequency Analyzer\n"
    "Finds what frequency a\n"
    "remote is on. Hold the\n"
    "remote, then hold OK to\n"
    "carry that frequency over\n"
    "to Read. Left and Right\n"
    "move the trigger level.\n"
    "\n"
    "\e#The second line\n"
    "Under the protocol name\n"
    "and bit count: whether the\n"
    "Sub-GHz app knows that\n"
    "protocol too, or whether it\n"
    "is analyzer only.\n";
/* clang-format on */

/** One manual at a time, and each scene entry maps a fresh copy of the plugin. */
static Widget* ra_about_widget = NULL;

static void ra_about_plugin_on_enter(RaAboutEnv* env) {
    furi_assert(env);
    furi_assert(!ra_about_widget);

    ra_about_widget = widget_alloc();
    widget_add_text_scroll_element(ra_about_widget, 0, 0, 128, 64, ra_about_text);
    view_dispatcher_add_view(env->view_dispatcher, env->view_id, widget_get_view(ra_about_widget));
    view_dispatcher_switch_to_view(env->view_dispatcher, env->view_id);
}

static void ra_about_plugin_on_exit(RaAboutEnv* env) {
    furi_assert(env);
    furi_assert(ra_about_widget);

    /* Park on an app-owned view before dropping ours: removing the current view leaves the
     * dispatcher with none, which stops the event loop for good. */
    view_dispatcher_switch_to_view(env->view_dispatcher, env->park_view_id);
    view_dispatcher_remove_view(env->view_dispatcher, env->view_id);

    widget_free(ra_about_widget);
    ra_about_widget = NULL;
}

static const RaAboutPlugin ra_about_plugin = {
    .on_enter = ra_about_plugin_on_enter,
    .on_exit = ra_about_plugin_on_exit,
};

static const FlipperAppPluginDescriptor ra_about_plugin_descriptor = {
    .appid = RA_ABOUT_PLUGIN_APP_ID,
    .ep_api_version = RA_ABOUT_PLUGIN_API_VERSION,
    .entry_point = &ra_about_plugin,
};

const FlipperAppPluginDescriptor* remote_analyzer_about_ep(void) {
    return &ra_about_plugin_descriptor;
}
