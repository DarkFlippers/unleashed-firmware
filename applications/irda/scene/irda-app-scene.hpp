#pragma once
#include "../irda-app-event.hpp"
#include <api-hal-irda.h>
#include "irda.h"
#include <vector>
#include <string>
#include "../irda-app-brute-force.hpp"


class IrdaApp;

class IrdaAppScene {
public:
    virtual void on_enter(IrdaApp* app) = 0;
    virtual bool on_event(IrdaApp* app, IrdaAppEvent* event) = 0;
    virtual void on_exit(IrdaApp* app) = 0;
    virtual ~IrdaAppScene(){};

private:
};

class IrdaAppSceneStart : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
private:
    uint32_t submenu_item_selected = 0;
};

class IrdaAppSceneUniversal : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
private:
    uint32_t submenu_item_selected = 0;
};

class IrdaAppSceneLearn : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
};

class IrdaAppSceneLearnSuccess : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
};

class IrdaAppSceneLearnEnterName : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
};

class IrdaAppSceneLearnDone : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
};

class IrdaAppSceneRemote : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
private:
    std::vector<std::string> buttons_names;
    uint32_t buttonmenu_item_selected = 0;
};

class IrdaAppSceneRemoteList : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
private:
    uint32_t submenu_item_selected = 0;
    std::vector<std::string> remote_names;
};

class IrdaAppSceneEdit : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
private:
    uint32_t submenu_item_selected = 0;
};

class IrdaAppSceneEditKeySelect : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
private:
    std::vector<std::string> buttons_names;
};

class IrdaAppSceneEditRename : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
};

class IrdaAppSceneEditDelete : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
};

class IrdaAppSceneEditRenameDone : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
};

class IrdaAppSceneEditDeleteDone : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
};

class IrdaAppSceneUniversalCommon : public IrdaAppScene {
    bool brute_force_started = false;
protected:
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
    IrdaAppBruteForce brute_force;
    void remove_popup(IrdaApp* app);
    void show_popup(IrdaApp* app, int record_amount);
    void progress_popup(IrdaApp* app);
    static void irda_app_item_callback(void* context, uint32_t index);
    IrdaAppSceneUniversalCommon(const char* filename) : brute_force(filename) {}
    ~IrdaAppSceneUniversalCommon() {}
};

class IrdaAppSceneUniversalTV : public IrdaAppSceneUniversalCommon {
public:
    void on_enter(IrdaApp* app) final;
    IrdaAppSceneUniversalTV() : IrdaAppSceneUniversalCommon("/assets/ext/irda/tv.ir") {}
    ~IrdaAppSceneUniversalTV() {}
};

class IrdaAppSceneUniversalAudio : public IrdaAppSceneUniversalCommon {
public:
    void on_enter(IrdaApp* app) final;
    IrdaAppSceneUniversalAudio() : IrdaAppSceneUniversalCommon("/assets/ext/irda/audio.ir") {}
    ~IrdaAppSceneUniversalAudio() {}
};

