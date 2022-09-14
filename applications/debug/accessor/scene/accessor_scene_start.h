#pragma once
#include "accessor_scene_generic.h"

class AccessorSceneStart : public AccessorScene {
public:
    void on_enter(AccessorApp* app) final;
    bool on_event(AccessorApp* app, AccessorEvent* event) final;
    void on_exit(AccessorApp* app) final;
};
