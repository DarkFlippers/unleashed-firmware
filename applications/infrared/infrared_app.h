/**
  * @file infrared_app.h
  * Infrared: Main infrared application class
  */
#pragma once
#include <map>
#include <infrared.h>
#include <furi.h>
#include <forward_list>
#include <stdint.h>
#include <notification/notification_messages.h>
#include <dialogs/dialogs.h>
#include <infrared_worker.h>

#include "scene/infrared_app_scene.h"
#include "scene/infrared_app_scene.h"
#include "infrared_app_view_manager.h"
#include "infrared_app_remote_manager.h"
#include "infrared_app_view_manager.h"

/** Main Infrared application class */
class InfraredApp {
public:
    /** Enum to save scene state: edit element */
    enum class EditElement : uint8_t {
        Button,
        Remote,
    };
    /** Enum to save scene state: edit action */
    enum class EditAction : uint8_t {
        Rename,
        Delete,
    };
    /** List of scenes for Infrared application */
    enum class Scene : uint8_t {
        Exit,
        Start,
        Universal,
        UniversalTV,
        UniversalAudio,
        UniversalAirConditioner,
        Learn,
        LearnSuccess,
        LearnEnterName,
        LearnDone,
        AskBack,
        Remote,
        RemoteList,
        Edit,
        EditKeySelect,
        EditRename,
        EditDelete,
        EditRenameDone,
        EditDeleteDone,
    };

    /** Start application
 *
 * @param args - application arguments.
 *      Allowed argument is path to remote file.
 * @retval 0 on success, error code otherwise
 */
    int32_t run(void* args);

    /** Switch to next scene. Put current scene number on stack.
 * Doesn't save scene state.
 *
 * @param index - next scene index
 */
    void switch_to_next_scene(Scene index);

    /** Switch to next scene, but don't put current scene on
 * stack. Thus calling switch_to_previous_scene() doesn't return
 * to current scene.
 *
 * @param index - next scene index
 */
    void switch_to_next_scene_without_saving(Scene index);

    /** Switch to previous scene. Pop scenes from stack and switch to last one.
 *
 * @param count - how many scenes should be popped
 * @retval false on failed, true on success
 */
    bool switch_to_previous_scene(uint8_t count = 1);

    /** Get previous scene in scene stack
 *
 * @retval previous scene
 */
    Scene get_previous_scene();

    /** Get view manager instance
 *
 * @retval view manager instance
 */
    InfraredAppViewManager* get_view_manager();

    /** Set one of text stores
 *
 * @param index - index of text store
 * @param text - text to set
 */
    void set_text_store(uint8_t index, const char* text...);

    /** Get value in text store
 *
 * @param index - index of text store
 * @retval value in text_store
 */
    char* get_text_store(uint8_t index);

    /** Get text store size
 *
 * @retval size of text store
 */
    uint8_t get_text_store_size();

    /** Get remote manager instance
 *
 * @retval remote manager instance
 */
    InfraredAppRemoteManager* get_remote_manager();

    /** Get infrared worker instance
 *
 * @retval infrared worker instance
 */
    InfraredWorker* get_infrared_worker();

    /** Get signal, previously got on Learn scene
 *
 * @retval received signal
 */
    const InfraredAppSignal& get_received_signal() const;

    /** Set received signal
 *
 * @param signal - signal
 */
    void set_received_signal(const InfraredAppSignal& signal);

    /** Switch to previous scene in one of provided in list.
 * Pop scene stack, and find first scene from list.
 *
 * @param scenes_list - list of scenes
 */
    void search_and_switch_to_previous_scene(const std::initializer_list<Scene>& scenes_list);

    /** Set edit element value. It is used on edit scene to determine
 * what should be deleted - remote or button.
 *
 * @param value - value to set
 */
    void set_edit_element(EditElement value);

    /** Get edit element
 *
 * @retval edit element value
 */
    EditElement get_edit_element(void);

    /** Set edit action value. It is used on edit scene to determine
 * what action to perform - deletion or renaming.
 *
 * @param value - value to set
 */
    void set_edit_action(EditAction value);

    /** Get edit action
 *
 * @retval edit action value
 */
    EditAction get_edit_action(void);

    /** Get state of learning new signal.
 * Adding new remote with 1 button from start scene and
 * learning 1 additional button to remote have very similar
 * flow, so they are joined. Difference in flow is handled
 * by this boolean flag.
 *
 * @retval false if flow is in learning new remote, true if
 *      adding signal to created remote
 *
 */
    bool get_learn_new_remote();

    /** Set state of learning new signal.
 * Adding new remote with 1 button from start scene and
 * learning 1 additional button to remote have very similar
 * flow, so they are joined. Difference in flow is handled
 * by this boolean flag.
 *
 * @param value - false if flow is in learning new remote, true if
 *      adding signal to created remote
 */
    void set_learn_new_remote(bool value);

    /** Button is not assigned value
 */
    enum : int {
        ButtonNA = -1,
    };

    /** Get current button index
 *
 * @retval current button index
 */
    int get_current_button();

    /** Set current button index
 *
 * @param current button index
 */
    void set_current_button(int value);

    /** Play success notification */
    void notify_success();
    /** Play red blink notification */
    void notify_blink_read();
    /** Light green */
    void notify_green_on();
    /** Disable green light */
    void notify_green_off();
    /** Blink on send */
    void notify_blink_send();

    /** Get Dialogs instance */
    DialogsApp* get_dialogs();

    /** Text input callback
 *
 * @param context - context to pass to callback
 */
    static void text_input_callback(void* context);

    /** Popup callback
 *
 * @param context - context to pass to callback
 */
    static void popup_callback(void* context);

    /** Signal sent callback
 *
 * @param context - context to pass to callback
 */
    static void signal_sent_callback(void* context);

    /** Main class constructor, initializes all critical objects */
    InfraredApp();
    /** Main class destructor, deinitializes all critical objects */
    ~InfraredApp();

    /** Path to Infrared directory */
    static constexpr const char* infrared_directory = "/any/infrared";
    /** Infrared files extension (remote files and universal databases) */
    static constexpr const char* infrared_extension = ".ir";
    /** Max Raw timings in signal */
    static constexpr const uint32_t max_raw_timings_in_signal = 512;
    /** Max line length in Infrared file */
    static constexpr const uint32_t max_line_length =
        (9 + 1) * InfraredApp::max_raw_timings_in_signal + 100;

private:
    /** Text store size */
    static constexpr const uint8_t text_store_size = 128;
    /** Amount of text stores */
    static constexpr const uint8_t text_store_max = 2;
    /** Store text here, for some views, because they doesn't
 * hold ownership of text */
    char text_store[text_store_max][text_store_size + 1];
    /**
 * Flag to control adding new signal flow.
 * Adding new remote with 1 button from start scene and
 * learning 1 additional button to remote have very similar
 * flow, so they are joined. Difference in flow is handled
 * by this boolean flag.
 */
    bool learn_new_remote;
    /** Value to control edit scene */
    EditElement element;
    /** Value to control edit scene */
    EditAction action;
    /** Selected button index */
    uint32_t current_button;

    /** Notification instance */
    NotificationApp* notification;
    /** Dialogs instance */
    DialogsApp* dialogs;
    /** View manager instance */
    InfraredAppViewManager view_manager;
    /** Remote manager instance */
    InfraredAppRemoteManager remote_manager;
    /** Infrared worker instance */
    InfraredWorker* infrared_worker;
    /** Signal received on Learn scene */
    InfraredAppSignal received_signal;

    /** Stack of previous scenes */
    std::forward_list<Scene> previous_scenes_list;
    /** Now acting scene */
    Scene current_scene = Scene::Start;

    /** Map of index/scene objects */
    std::map<Scene, InfraredAppScene*> scenes = {
        {Scene::Start, new InfraredAppSceneStart()},
        {Scene::Universal, new InfraredAppSceneUniversal()},
        {Scene::UniversalTV, new InfraredAppSceneUniversalTV()},
        {Scene::Learn, new InfraredAppSceneLearn()},
        {Scene::LearnSuccess, new InfraredAppSceneLearnSuccess()},
        {Scene::LearnEnterName, new InfraredAppSceneLearnEnterName()},
        {Scene::LearnDone, new InfraredAppSceneLearnDone()},
        {Scene::AskBack, new InfraredAppSceneAskBack()},
        {Scene::Remote, new InfraredAppSceneRemote()},
        {Scene::RemoteList, new InfraredAppSceneRemoteList()},
        {Scene::Edit, new InfraredAppSceneEdit()},
        {Scene::EditKeySelect, new InfraredAppSceneEditKeySelect()},
        {Scene::EditRename, new InfraredAppSceneEditRename()},
        {Scene::EditDelete, new InfraredAppSceneEditDelete()},
        {Scene::EditRenameDone, new InfraredAppSceneEditRenameDone()},
        {Scene::EditDeleteDone, new InfraredAppSceneEditDeleteDone()},
    };
};
