#pragma once
#include "../accessor_app.h"

class AccessorApp;

class AccessorScene {
public:
    virtual void on_enter(AccessorApp* app) = 0;
    virtual bool on_event(AccessorApp* app, AccessorEvent* event) = 0;
    virtual void on_exit(AccessorApp* app) = 0;

private:
};
