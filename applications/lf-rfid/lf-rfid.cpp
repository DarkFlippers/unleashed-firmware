#include "lf-rfid-app.h"

// app enter function
extern "C" int32_t app_lfrfid(void* p) {
    LfrfidApp* app = new LfrfidApp();
    app->run();
    delete app;

    return 255;
}