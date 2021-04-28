#pragma once
#include "accessor-scene-generic.h"

class AccessorSceneStart : public AccessorScene {
public:
    void on_enter(AccessorApp* app) final;
    bool on_event(AccessorApp* app, AccessorEvent* event) final;
    void on_exit(AccessorApp* app) final;
};