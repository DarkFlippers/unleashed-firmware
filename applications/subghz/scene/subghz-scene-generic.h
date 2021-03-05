#pragma once
#include "../subghz-event.h"

class SubghzApp;

class SubghzScene {
public:
    virtual void on_enter(SubghzApp* app) = 0;
    virtual bool on_event(SubghzApp* app, SubghzEvent* event) = 0;
    virtual void on_exit(SubghzApp* app) = 0;

private:
};