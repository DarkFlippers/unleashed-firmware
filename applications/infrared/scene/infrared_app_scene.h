/**
 * @file infrared_app_scene.h
 * Infrared: Application scenes
 */
#pragma once
#include "../infrared_app_event.h"
#include <furi_hal_infrared.h>
#include "infrared.h"
#include <vector>
#include <string>
#include "../infrared_app_brute_force.h"

/** Anonymous class */
class InfraredApp;

/** Base Scene class */
class InfraredAppScene {
public:
    /** Called when enter scene */
    virtual void on_enter(InfraredApp* app) = 0;
    /** Events handler callback */
    virtual bool on_event(InfraredApp* app, InfraredAppEvent* event) = 0;
    /** Called when exit scene */
    virtual void on_exit(InfraredApp* app) = 0;
    /** Virtual destructor of base class */
    virtual ~InfraredAppScene(){};

private:
};

/** Start scene
 * Main Infrared application menu
 */
class InfraredAppSceneStart : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;

private:
    /** Save previously selected submenu index
     * to highlight it when get back */
    uint32_t submenu_item_selected = 0;
};

/** Universal menu scene
 * Scene to select universal remote
 */
class InfraredAppSceneUniversal : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;

private:
    /** Save previously selected submenu index
     * to highlight it when get back */
    uint32_t submenu_item_selected = 0;
};

/** Learn new signal scene
 * On this scene catching new IR signal performed.
 */
class InfraredAppSceneLearn : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;
};

/** New signal learn succeeded scene
 */
class InfraredAppSceneLearnSuccess : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;
    bool button_pressed = false;
};

/** Scene to enter name for new button in remote
 */
class InfraredAppSceneLearnEnterName : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;
};

/** Scene where signal is learnt
 */
class InfraredAppSceneLearnDone : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;
};

/** Remote interface scene
 * On this scene you can send IR signals from selected remote
 */
class InfraredAppSceneRemote : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;

private:
    /** container of button names in current remote. */
    std::vector<std::string> buttons_names;
    /** Save previously selected index
     * to highlight it when get back */
    uint32_t buttonmenu_item_selected = 0;
    /** state flag to show button is pressed.
     * As long as send-signal button pressed no other button
     * events are handled. */
    bool button_pressed = false;
};

/** List of remotes scene
 * Every remote is a file, located on internal/external storage.
 * Every file has same format, and same extension.
 * Files are parsed as you enter 'Remote scene' and showed
 * as a buttons.
 */
class InfraredAppSceneRemoteList : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;

private:
    /** Save previously selected index
     * to highlight it when get back */
    uint32_t submenu_item_selected = 0;
    /** Remote names to show them in submenu */
    std::vector<std::string> remote_names;
};

class InfraredAppSceneAskBack : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;
};

class InfraredAppSceneEdit : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;

private:
    /** Save previously selected index
     * to highlight it when get back */
    uint32_t submenu_item_selected = 0;
};

class InfraredAppSceneEditKeySelect : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;

private:
    /** Button names to show them in submenu */
    std::vector<std::string> buttons_names;
};

class InfraredAppSceneEditRename : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;
};

class InfraredAppSceneEditDelete : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;
};

class InfraredAppSceneEditRenameDone : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;
};

class InfraredAppSceneEditDeleteDone : public InfraredAppScene {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;
};

class InfraredAppSceneUniversalCommon : public InfraredAppScene {
    /** Brute force started flag */
    bool brute_force_started = false;

protected:
    /** Events handler callback */
    bool on_event(InfraredApp* app, InfraredAppEvent* event) final;
    /** Called when exit scene */
    void on_exit(InfraredApp* app) final;

    /** Show popup window
     *
     * @param app - application instance
     */
    void show_popup(InfraredApp* app, int record_amount);

    /** Hide popup window
     *
     * @param app - application instance
     */
    void hide_popup(InfraredApp* app);

    /** Propagate progress in popup window
     *
     * @param app - application instance
     */
    bool progress_popup(InfraredApp* app);

    /** Item selected callback
     *
     * @param context - context
     * @param index - selected item index
     */
    static void infrared_app_item_callback(void* context, uint32_t index);

    /** Brute Force instance */
    InfraredAppBruteForce brute_force;

    /** Constructor */
    InfraredAppSceneUniversalCommon(const char* filename)
        : brute_force(filename) {
    }

    /** Destructor */
    ~InfraredAppSceneUniversalCommon() {
    }
};

class InfraredAppSceneUniversalTV : public InfraredAppSceneUniversalCommon {
public:
    /** Called when enter scene */
    void on_enter(InfraredApp* app) final;

    /** Constructor
     * Specifies path to brute force db library */
    InfraredAppSceneUniversalTV()
        : InfraredAppSceneUniversalCommon("/ext/infrared/assets/tv.ir") {
    }

    /** Destructor */
    ~InfraredAppSceneUniversalTV() {
    }
};
