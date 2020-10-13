#pragma once

#include "flipper.h"

// == Flipper Application control (flapp) ==

typedef FlappHandler uint32_t; // TODO

/*
simply starts application. It call `app` entrypoint with `param` passed as argument
Useful for daemon applications and pop-up.
*/
FlappHandler* flapp_start(void(app*)(void*), char* name, void* param);

/*
swtich to other application.
System **stop current app**, call `app` entrypoint with `param` passed
as argument and save current application entrypoint to `prev` field in
current application registry. Useful for UI or "active" application.
*/
FlappHandler* flapp_switch(void(app*)(void*), char* name, void* param);

/*
Exit application
stop current application (stop thread and clear application's stack),
start application from `prev` entry in current application registry,
cleanup current application registry.
*/
void flapp_exit(void* param);

/*
stop specified `app` without returning to `prev` application.
*/
bool flapp_kill(FlappHandler* app);

/*
If case one app depend on other, notify that app is ready.
*/
void flapp_ready();

/*
Register on-exit callback.
It called before app will be killed.
Not recommended to use in user scenario, only for system purpose
(unregister callbacks, release mutexes, etc.)
*/
bool flapp_on_exit(void(cb*)(void*), void* ctx);
