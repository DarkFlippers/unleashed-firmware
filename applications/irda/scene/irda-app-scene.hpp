#pragma once
#include "../irda-app.hpp"
#include <api-hal-irda.h>
#include "irda.h"
#include <gui/elements.h>
#include <vector>
#include <string>

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
};

class IrdaAppSceneUniversal : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
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

class IrdaAppSceneLearnDoneAfter : public IrdaAppScene {
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
};

class IrdaAppSceneRemoteList : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
    std::vector<std::string> remote_names;
};

class IrdaAppSceneEdit : public IrdaAppScene {
public:
    void on_enter(IrdaApp* app) final;
    bool on_event(IrdaApp* app, IrdaAppEvent* event) final;
    void on_exit(IrdaApp* app) final;
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

