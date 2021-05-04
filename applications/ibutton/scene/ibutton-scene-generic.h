#pragma once
#include "../ibutton-event.h"

class iButtonApp;

class iButtonScene {
public:
    virtual void on_enter(iButtonApp* app) = 0;
    virtual bool on_event(iButtonApp* app, iButtonEvent* event) = 0;
    virtual void on_exit(iButtonApp* app) = 0;
    virtual ~iButtonScene(){};

private:
};
