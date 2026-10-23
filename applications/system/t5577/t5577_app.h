/**
 * @file t5577_app.h
 * @brief Reading, saving and restoring T5577 transponders.
 */
#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/popup.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/widget.h>
#include <gui/modules/variable_item_list.h>
#include <dialogs/dialogs.h>
#include <notification/notification_messages.h>
#include <storage/storage.h>
#include <toolbox/keys_dict.h>

#include <lfrfid/tools/t5577.h>
#include <lfrfid/tools/t5577_file.h>

#include "views/t5577_dump_view.h"
#include "views/t5577_test_view.h"
#include "views/t5577_work_view.h"

#include "scenes/scenes.h"

// The lfrfid app's own folder, not one beside it: a dump of a tag belongs with the keys read
// off tags, and the lfrfid saved list shows both and hands these over to this app.
#define T5577_APP_FOLDER       EXT_PATH("lfrfid")
// This app's own folder inside it, which is not where the user's dumps go. The backup a block
// test writes lives here, being scratch with no business in a list of saved keys, and so do the
// dumps versions before this one saved - left where they are, and listed by the browser like
// any other subfolder of the app folder.
#define T5577_APP_OWN_FOLDER   EXT_PATH("lfrfid/t5577")
#define T5577_APP_NAME_MAX     32
#define T5577_APP_TEST_BACKUP  "T5577_before_test"
// Every block a dump holds that is worth typing into: the same set a write puts back, in
// reading order rather than writing order.
#define T5577_APP_EDIT_BLOCKS  T5577_RESTORE_BLOCKS
// The named fields of block 0, plus the word they add up to.
#define T5577_APP_CONFIG_ITEMS 5
#define T5577_APP_HEADER_MAX   18 // characters a header, or a line of dialog, has room for
// Blocks that would not read back before a read or a check gives up on the tag. A chip in
// the field that answered once does not go quiet for a second and a half and come back, and
// every block asked for after it has gone costs that long.
#define T5577_APP_LOST_MAX     2
// Blocks a test probes: page 0 without block 0, which it never touches.
#define T5577_APP_TEST_BLOCKS  (LFRFID_T5577_BLOCKS_IN_PAGE_0 - 1)

/** Why a block test stopped short of testing every block.
 *
 * Each one calls for something different of the user, which is why they are not one failure: a
 * tag that was never found wants re-seating, a protected tag is not supported by this version,
 * and a block that would not go back wants the backup written to the tag.
 */
typedef enum {
    T5577TestStopNone,
    T5577TestStopStopped, // the user pressed Back; the block in flight was still put back
    T5577TestStopNoTag,
    T5577TestStopPassword,
    T5577TestStopUnreadable,
    T5577TestStopNoBackup,
    T5577TestStopLost,
    T5577TestStopOdd,
    T5577TestStopNotRestored,
} T5577TestStop;

typedef struct {
    Gui* gui;
    Storage* storage;
    DialogsApp* dialogs;
    NotificationApp* notifications;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    Popup* popup;
    Submenu* submenu;
    TextInput* text_input;
    ByteInput* byte_input;
    Widget* widget;
    VariableItemList* config_list;
    VariableItem* config_item[T5577_APP_CONFIG_ITEMS];
    T5577DumpView* dump_view;
    T5577TestView* test_view;
    T5577WorkView* work_view;

    // What the tag said, and how it said it.
    T5577Dump dump;
    T5577Config config;

    // A password to try, which is not the same as one that worked: it moves into the dump only
    // once the tag has answered with it.
    uint32_t password;
    // What a restore is addressing its writes to, which changes under it once block 7 is written.
    bool password_entered;
    uint8_t password_bytes[4]; // what the byte input edits in place
    uint8_t edit_bytes[4]; // and what it edits when the subject is a block
    // The dump as the file on the card holds it. Kept so that "has this been edited" is a
    // comparison rather than a flag some scene has to remember to set, and so that declining to
    // save is a copy that cannot fail rather than a re-read that can.
    T5577Dump saved;

    // A rename is a save under a new name followed by a delete of the old one, in that order.
    bool renaming;
    // Started on a dump another app handed over: the file menu is then the whole visit, and
    // leaving it leaves the app, back to wherever the file was picked.
    bool file_only;

    FuriString* path;
    char name[T5577_APP_NAME_MAX];

    // One step of a read or a restore per timer tick, so the screen stays answerable and Back
    // still works between blocks. A block takes most of a second, and there are twelve.
    FuriTimer* step_timer;
    uint8_t step; // where the scene has got to, its own meaning per scene
    uint8_t page; // block a read is on
    uint8_t block;
    uint8_t order; // place in the write order a restore is on
    uint8_t done;
    uint8_t checked; // blocks a restore has read back, for the second bar
    uint8_t bad; // blocks that read back as something else
    // Counted apart from `bad`, never into it: the traceability blocks are locked at
    // manufacture, so counting a refusal there as a failure would report every clone of a dump
    // onto a genuine blank as broken.
    uint8_t bad_trace;
    uint8_t lost; // blocks that would not read back at all
    // One target per kind, not one for whichever came first: a block that read back different
    // and a block that went quiet are different diagnoses, and naming one of them with the
    // other's block is how a locked block gets reported as a tag that moved.
    T5577Target lost_at;
    T5577Target bad_at;
    // Blocks a test's write landed on, one bit per page 0 block. A block it has not reached
    // yet is simply absent; what became of the ones it did reach is on screen a cell at a time.
    uint8_t test_writeable;
    T5577TestStop test_stop; // why it stopped short, or None
    // Whether this run is the one that wrote the backup. A run that stops before the first
    // write leaves an older backup alone: a fixed name makes every run's backup look alike,
    // and that one is some other tag's way home.
    bool test_backed_up;
    // Clear Password: how far the sweep has got, how many candidates there are, and whether one
    // of them opened the tag.
    uint16_t attempt;
    uint16_t candidates;
    bool dict_failed; // the list stopped answering part way, which is not an absent list
    KeysDict* dict; // open only while a sweep is running
    bool cleared;
    T5577BlockResult wipe_result; // what the wipe's read back came to
    char progress[40]; // the popup keeps this by pointer, so it outlives the call
} T5577App;

typedef enum {
    T5577AppViewPopup,
    T5577AppViewSubmenu,
    T5577AppViewDump,
    T5577AppViewTest,
    T5577AppViewWork,
    T5577AppViewTextInput,
    T5577AppViewByteInput,
    T5577AppViewWidget,
    T5577AppViewConfig,
} T5577AppView;

typedef enum {
    T5577AppCustomEventStep = 100, // the step timer fired
    T5577AppCustomEventSave,
    T5577AppCustomEventInfo,
    T5577AppCustomEventRetry,
    T5577AppCustomEventRetryPlain, // retry, forgetting the password that was entered
    T5577AppCustomEventPassword,
    T5577AppCustomEventConfirm,
    T5577AppCustomEventOpen, // a dump handed over by another app is waiting in app->path
} T5577AppCustomEvent;

/** Arm the step timer for one more step of whatever the current scene is doing. */
void t5577_app_step_later(T5577App* app, uint32_t delay_ms);

/** Stop the step timer, for a scene being left while a step is still pending. */
void t5577_app_step_stop(T5577App* app);

/** Forget a password that was typed in, so the next read does not send it.
 *
 * A password frame is read off the wrong bit positions by a chip that has none, which writes its
 * block 0 - so a password that did not work is worth dropping rather than retrying with.
 */
void t5577_app_forget_password(T5577App* app);

/** The dump's filename, cut to what a header or a line of dialog text has room for. */
void t5577_app_short_name(const T5577App* app, FuriString* name);

/** Say something and wait for OK. */
void t5577_app_say(T5577App* app, const char* header, const char* text);

/** Ask before doing something to a tag or a file. True when the right button was pressed. */
bool t5577_app_ask(T5577App* app, const char* header, const char* text, const char* yes);

/** Load the dump at app->path and keep it as the saved copy to compare edits against.
 *
 * False when it would not load, which is said on screen before returning: a dump is opened from
 * the browser and from other apps, and both want the same answer when the file is not one.
 */
bool t5577_app_open_dump(T5577App* app);

/** Where a test's backup lives. One name for every run, so it is built in one place. */
void t5577_app_test_backup_path(FuriString* path);

/** Leave the app from whatever scene is showing. */
void t5577_app_exit(T5577App* app);

/** The block the editor is on, which the scenes it opens need and do not choose. */
T5577Target t5577_app_edit_block(const T5577App* app);
