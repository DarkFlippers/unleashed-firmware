#pragma once
#include "../lf-rfid-event.h"

class LfrfidApp;

class LfrfidScene {
public:
    virtual void on_enter(LfrfidApp* app) = 0;
    virtual bool on_event(LfrfidApp* app, LfrfidEvent* event) = 0;
    virtual void on_exit(LfrfidApp* app) = 0;
    virtual ~LfrfidScene(){};

private:
};