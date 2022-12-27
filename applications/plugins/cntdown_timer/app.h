#ifndef __APP_H__
#define __APP_H__

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>

// app
typedef struct {
    Gui* gui; // gui object
    ViewDispatcher* view_dispatcher; // view dispacther of the gui

    // views
    CountDownTimView* helloworld_view;

} CountDownTimerApp;

CountDownTimerApp* countdown_app_new(void);
void countdown_app_delete(CountDownTimerApp* app);
void countdown_app_run(CountDownTimerApp* app);

#endif